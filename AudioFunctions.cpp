// Include order matters for these; don't let the autoformatter break things

// clang-format off
#include "windows.h"
#include "endpointvolume.h"
#include "mmdeviceapi.h"
#include "PolicyConfig.h"
#include "Functiondiscoverykeys_devpkey.h"
// clang-format on

#include "AudioFunctions.h"

#include <atlbase.h>
#include <cassert>
#include <codecvt>
#include <locale>
#include <comip.h>

namespace {

[[noreturn]] void assume_unreachable() {
  abort();
}

//std::string WCharPtrToString(LPCWSTR in) {
//  if (!in) {
//    return std::string();
//  }
//  int utf8_len = WideCharToMultiByte(CP_UTF8, 0, in, -1, 0, 0, 0, 0);
//  std::string buf(utf8_len, 0);
//  WideCharToMultiByte(CP_UTF8, 0, in, -1, buf.data(), utf8_len, 0, 0);
//  buf.resize(utf8_len - 1);
//  return buf;
//}

//std::wstring Utf8StrToWString(const std::string& in) {
//  int wchar_len
//    = MultiByteToWideChar(CP_UTF8, 0, in.c_str(), in.size(), 0, 0);
//  std::wstring buf(wchar_len, 0);
//  MultiByteToWideChar(CP_UTF8, 0, in.c_str(), in.size(), buf.data(), wchar_len);
//  return buf;
//}

// Convert a wide Unicode string to an UTF8 string
std::string WCharPtrToString(const std::wstring &wstr)
{
    if( wstr.empty() ) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring Utf8StrToWString(const std::string &str)
{
    if( str.empty() ) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

EDataFlow AudioDeviceDirectionToEDataFlow(const AudioDeviceDirection dir) {
  switch (dir) {
    case AudioDeviceDirection::INPUT:
      return eCapture;
    case AudioDeviceDirection::OUTPUT:
      return eRender;
  }
  __assume(0);
}

ERole AudioDeviceRoleToERole(const AudioDeviceRole role) {
  switch (role) {
    case AudioDeviceRole::COMMUNICATION:
      return eCommunications;
    case AudioDeviceRole::DEFAULT:
      return eConsole;
  }
  __assume(0);
}

CComPtr<IMMDevice> DeviceIDToDevice(const std::string& in) {
  CComPtr<IMMDeviceEnumerator> de;
  de.CoCreateInstance(__uuidof(MMDeviceEnumerator));
  CComPtr<IMMDevice> device;
  de->GetDevice(Utf8StrToWString(in).data(), &device);
  return device;
}

CComPtr<IAudioEndpointVolume> DeviceIDToAudioEndpointVolume(
  const std::string& deviceID) {
  auto device = DeviceIDToDevice(deviceID);
  if (!device) {
    return nullptr;
  }
  CComPtr<IAudioEndpointVolume> volume;
  device->Activate(
    __uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&volume);
  return volume;
}

AudioDeviceState GetAudioDeviceState(CComPtr<IMMDevice> device) {
  DWORD nativeState;
  device->GetState(&nativeState);

  switch (nativeState) {
    case DEVICE_STATE_ACTIVE:
      return AudioDeviceState::CONNECTED;
    case DEVICE_STATE_DISABLED:
      return AudioDeviceState::DEVICE_DISABLED;
    case DEVICE_STATE_NOTPRESENT:
      return AudioDeviceState::DEVICE_NOT_PRESENT;
    case DEVICE_STATE_UNPLUGGED:
      return AudioDeviceState::DEVICE_PRESENT_NO_CONNECTION;
  }
  assume_unreachable();
}

}// namespace

AudioDeviceState GetAudioDeviceState(const std::string& id) {
  auto device = DeviceIDToDevice(id);
  if (device == nullptr) {
    return AudioDeviceState::DEVICE_NOT_PRESENT;
  }
  return GetAudioDeviceState(device);
}

std::map<std::string, AudioDeviceInfo> GetAudioDeviceList(
  AudioDeviceDirection direction) {
  CComPtr<IMMDeviceEnumerator> de;
  de.CoCreateInstance(__uuidof(MMDeviceEnumerator));

  CComPtr<IMMDeviceCollection> devices;
  de->EnumAudioEndpoints(
    AudioDeviceDirectionToEDataFlow(direction), DEVICE_STATEMASK_ALL, &devices);

  UINT deviceCount;
  devices->GetCount(&deviceCount);
  std::map<std::string, AudioDeviceInfo> out;

  for (UINT i = 0; i < deviceCount; ++i) {
    CComPtr<IMMDevice> device;
    devices->Item(i, &device);
    LPWSTR nativeID;
    device->GetId(&nativeID);
    const auto id = WCharPtrToString(nativeID);
    CComPtr<IPropertyStore> properties;
    device->OpenPropertyStore(STGM_READ, &properties);
    PROPVARIANT nativeCombinedName;
    properties->GetValue(PKEY_Device_FriendlyName, &nativeCombinedName);
    PROPVARIANT nativeInterfaceName;
    properties->GetValue(
      PKEY_DeviceInterface_FriendlyName, &nativeInterfaceName);
    PROPVARIANT nativeEndpointName;
    properties->GetValue(PKEY_Device_DeviceDesc, &nativeEndpointName);

    if (!nativeCombinedName.pwszVal) {
      continue;
    }

    out[id] = AudioDeviceInfo{
      id,
      WCharPtrToString(nativeInterfaceName.pwszVal),
      WCharPtrToString(nativeEndpointName.pwszVal),
      WCharPtrToString(nativeCombinedName.pwszVal),
      direction,
      GetAudioDeviceState(device)};
  }
  return out;
}

std::string GetDefaultAudioDeviceID(
  AudioDeviceDirection direction,
  AudioDeviceRole role) {
  CComPtr<IMMDeviceEnumerator> de;
  de.CoCreateInstance(__uuidof(MMDeviceEnumerator));
  if (!de) {
    //ESDDebug("Failed to create MMDeviceEnumerator");
    return std::string();
  }
  CComPtr<IMMDevice> device;
  de->GetDefaultAudioEndpoint(
    AudioDeviceDirectionToEDataFlow(direction), AudioDeviceRoleToERole(role),
    &device);
  if (!device) {
    //ESDDebug("No default audio device");
    return std::string();
  }
  LPWSTR deviceID;
  device->GetId(&deviceID);
  if (!deviceID) {
    //ESDDebug("No default audio device ID");
    return std::string();
  }
  return WCharPtrToString(deviceID);
}

void SetDefaultAudioDeviceID(
  AudioDeviceDirection direction,
  AudioDeviceRole role,
  const std::string& desiredID) {
  if (desiredID == GetDefaultAudioDeviceID(direction, role)) {
    return;
  }

  CComPtr<IPolicyConfigVista> pPolicyConfig;
  pPolicyConfig.CoCreateInstance(__uuidof(CPolicyConfigVistaClient));
  pPolicyConfig->SetDefaultEndpoint(
    Utf8StrToWString(desiredID).c_str(), AudioDeviceRoleToERole(role));
}

bool IsAudioDeviceMuted(const std::string& deviceID) {
  auto volume = DeviceIDToAudioEndpointVolume(deviceID);
  if (!volume) {
    return false;
  }
  BOOL ret;
  volume->GetMute(&ret);
  return ret;
}

void SetIsAudioDeviceMuted(const std::string& deviceID, MuteAction action) {
  auto volume = DeviceIDToAudioEndpointVolume(deviceID);
  if (!volume) {
    return;
  }
  if (action == MuteAction::MUTE) {
    volume->SetMute(true, nullptr);
  } else if (action == MuteAction::UNMUTE) {
    volume->SetMute(false, nullptr);
  } else {
    assert(action == MuteAction::TOGGLE);
    BOOL muted;
    volume->GetMute(&muted);
    volume->SetMute(!muted, nullptr);
  }
}

namespace {
class VolumeCallback : public IAudioEndpointVolumeCallback {
 public:
  VolumeCallback(std::function<void(bool isMuted)> cb) : mCB(cb), mRefs(1) {
  }

  virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ret)
    override {
    if (iid == IID_IUnknown || iid == __uuidof(IAudioEndpointVolumeCallback)) {
      *ret = static_cast<IUnknown*>(this);
      AddRef();
      return S_OK;
    }
    *ret = nullptr;
    return E_NOINTERFACE;
  }

  virtual ULONG __stdcall AddRef() override {
    return InterlockedIncrement(&mRefs);
  }
  virtual ULONG __stdcall Release() override {
    if (InterlockedDecrement(&mRefs) == 0) {
      delete this;
    }
    return mRefs;
  }
  virtual HRESULT OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override {
    mCB(pNotify->bMuted);
    return S_OK;
  }

 private:
  std::function<void(bool)> mCB;
  long mRefs;
};

struct VolumeCallbackHandle {
  std::string deviceID;
  CComPtr<VolumeCallback> impl;
  CComPtr<IAudioEndpointVolume> dev;

  VolumeCallbackHandle(
    const std::string& deviceID,
    CComPtr<VolumeCallback> impl,
    CComPtr<IAudioEndpointVolume> dev) {
    this->deviceID = deviceID;
    this->impl = impl;
    this->dev = dev;
  }

  VolumeCallbackHandle(const VolumeCallback& copied) = delete;

  ~VolumeCallbackHandle() {
    dev->UnregisterControlChangeNotify(impl);
  }
};

}// namespace

AUDIO_DEVICE_MUTE_CALLBACK_HANDLE
AddAudioDeviceMuteUnmuteCallback(
  const std::string& deviceID,
  std::function<void(bool isMuted)> cb) {
  auto dev = DeviceIDToAudioEndpointVolume(deviceID);
  if (!dev) {
    return nullptr;
  }
  auto impl = new VolumeCallback(cb);
  auto ret = dev->RegisterControlChangeNotify(impl);
  if (ret != S_OK) {
    delete impl;
    return nullptr;
  }
  return new VolumeCallbackHandle(deviceID, impl, dev);
}

void RemoveAudioDeviceMuteUnmuteCallback(
  AUDIO_DEVICE_MUTE_CALLBACK_HANDLE _handle) {
  if (!_handle) {
    return;
  }
  const auto handle = reinterpret_cast<VolumeCallbackHandle*>(_handle);
  delete handle;
  return;
}

namespace {
typedef std::function<
  void(AudioDeviceDirection, AudioDeviceRole, const std::string&)>
  DefaultChangeCallbackFun;
class DefaultChangeCallback : public IMMNotificationClient {
 public:
  DefaultChangeCallback(DefaultChangeCallbackFun cb) : mCB(cb), mRefs(1) {
  }

  virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ret)
    override {
    if (iid == IID_IUnknown || iid == __uuidof(IMMNotificationClient)) {
      *ret = static_cast<IUnknown*>(this);
      AddRef();
      return S_OK;
    }
    *ret = nullptr;
    return E_NOINTERFACE;
  }

  virtual ULONG __stdcall AddRef() override {
    return InterlockedIncrement(&mRefs);
  }
  virtual ULONG __stdcall Release() override {
    if (InterlockedDecrement(&mRefs) == 0) {
      delete this;
    }
    return mRefs;
  }

  virtual HRESULT OnDefaultDeviceChanged(
    EDataFlow flow,
    ERole winAudioDeviceRole,
    LPCWSTR defaultDeviceID) override {
    AudioDeviceRole role;
    switch (winAudioDeviceRole) {
      case ERole::eMultimedia:
        return S_OK;
      case ERole::eCommunications:
        role = AudioDeviceRole::COMMUNICATION;
        break;
      case ERole::eConsole:
        role = AudioDeviceRole::DEFAULT;
        break;
    }
    const AudioDeviceDirection direction = (flow == EDataFlow::eCapture)
                                             ? AudioDeviceDirection::INPUT
                                             : AudioDeviceDirection::OUTPUT;
    mCB(direction, role, WCharPtrToString(defaultDeviceID));

    return S_OK;
  };

  virtual HRESULT OnDeviceAdded(LPCWSTR pwstrDeviceId) override {
    return S_OK;
  };

  virtual HRESULT OnDeviceRemoved(LPCWSTR pwstrDeviceId) override {
    return S_OK;
  };

  virtual HRESULT OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
    override {
    return S_OK;
  };

  virtual HRESULT OnPropertyValueChanged(
    LPCWSTR pwstrDeviceId,
    const PROPERTYKEY key) override {
    return S_OK;
  };

 private:
  DefaultChangeCallbackFun mCB;
  long mRefs;
};

struct DefaultChangeCallbackHandle {
  CComPtr<DefaultChangeCallback> impl;
  CComPtr<IMMDeviceEnumerator> enumerator;

  DefaultChangeCallbackHandle(
    CComPtr<DefaultChangeCallback> impl,
    CComPtr<IMMDeviceEnumerator> enumerator) {
    this->impl = impl;
    this->enumerator = enumerator;
  }

  DefaultChangeCallbackHandle(const DefaultChangeCallbackHandle& copied)
    = delete;

  ~DefaultChangeCallbackHandle() {
    //ESDDebug("unregistering native callback");
    enumerator->UnregisterEndpointNotificationCallback(this->impl);
  }
};

}// namespace

DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE
AddDefaultAudioDeviceChangeCallback(DefaultChangeCallbackFun cb) {
  CComPtr<IMMDeviceEnumerator> de;
  de.CoCreateInstance(__uuidof(MMDeviceEnumerator));
  if (!de) {
    //ESDDebug("failed to get enumerator");
    return nullptr;
  }
  CComPtr<DefaultChangeCallback> impl(new DefaultChangeCallback(cb));
  if (de->RegisterEndpointNotificationCallback(impl) != S_OK) {
    //ESDDebug("failed to register callback");
    return nullptr;
  }

  //ESDDebug("returning new callback handle");
  return new DefaultChangeCallbackHandle(impl, de);
}

void RemoveDefaultAudioDeviceChangeCallback(
  DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE _handle) {
  if (!_handle) {
    return;
  }
  //ESDDebug("RemoveDefaultAudioDeviceChangeCallback called");
  const auto handle = reinterpret_cast<DefaultChangeCallbackHandle*>(_handle);
  delete handle;
  return;
}

#ifdef HAVE_FEEDBACK_SOUNDS
namespace {
std::wstring sMuteWav;
std::wstring sUnmuteWav;

void initWavPaths() {
  if (!sMuteWav.empty()) {
    return;
  }
  std::wstring path;
  path.resize(1024);
  const auto len = GetModuleFileName(NULL, path.data(), 1024);
  assert(len);
  path.resize(len);

  auto idx = path.find_last_of(L'\\');
  const auto dir = path.substr(0, idx + 1);
  auto utf8 = WCharPtrToString(dir.c_str());
  ESDDebug("got executable dir: '%s', %d", utf8.c_str(), idx);
  sMuteWav = dir + L"mute.wav";
  sUnmuteWav = dir + L"unmute.wav";
}

std::wstring muteWavPath() {
  initWavPaths();
  return sMuteWav;
}

std::wstring unmuteWavPath() {
  initWavPaths();
  return sUnmuteWav;
}
}// namespace

void PlayFeedbackSound(MuteAction action) {
  assert(action != MuteAction::TOGGLE);
  const auto feedbackWav
    = (action == MuteAction::MUTE) ? muteWavPath() : unmuteWavPath();
  const auto utf8 = WCharPtrToString(feedbackWav.c_str());
  ESDDebug("Playing feedback sound '%s'", utf8.c_str());
  PlaySound(feedbackWav.c_str(), NULL, SND_ASYNC | SND_NODEFAULT);
}
#endif
