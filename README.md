# MiniFilter
파일시스템 미니필터

## 개요 (Overview)

MiniFilter는 Windows 파일시스템 미니필터 드라이버입니다. 이 드라이버는 파일 시스템 작업(생성, 읽기, 쓰기 등)을 모니터링하고 필터링할 수 있습니다.

This is a Windows filesystem minifilter driver that can monitor and filter file system operations such as create, read, and write operations.

## 기능 (Features)

- 파일 생성(Create) 작업 모니터링
- 파일 읽기(Read) 작업 모니터링
- 파일 쓰기(Write) 작업 모니터링
- Pre-operation 및 Post-operation 콜백 지원
- 스트림 컨텍스트 지원

## 시스템 요구사항 (System Requirements)

- Windows 10 이상 (Windows 10 or later)
- Windows Driver Kit (WDK) 10
- Visual Studio 2015 이상 (Visual Studio 2015 or later)

## 빌드 방법 (Build Instructions)

### Visual Studio 사용

1. Visual Studio에서 `MiniFilter.sln` 파일 열기
2. 빌드 구성 선택 (Debug 또는 Release)
3. 플랫폼 선택 (x64, x86, ARM, ARM64)
4. 솔루션 빌드 (F7 또는 Build > Build Solution)

### 명령줄 빌드

```cmd
msbuild MiniFilter.sln /p:Configuration=Release /p:Platform=x64
```

## 설치 방법 (Installation)

### 드라이버 서명 (Driver Signing)

프로덕션 환경에서는 드라이버가 서명되어야 합니다. 테스트 목적으로는 테스트 서명을 활성화할 수 있습니다:

```cmd
bcdedit /set testsigning on
```

### 드라이버 설치

관리자 권한으로 명령 프롬프트를 실행하고:

```cmd
rundll32.exe setupapi,InstallHinfSection DefaultInstall 132 MiniFilter.inf
```

또는 Device Manager를 통해 INF 파일을 사용하여 설치할 수 있습니다.

## 드라이버 제어 (Driver Control)

### 드라이버 시작

```cmd
sc start MiniFilter
```

또는

```cmd
fltmc load MiniFilter
```

### 드라이버 중지

```cmd
sc stop MiniFilter
```

또는

```cmd
fltmc unload MiniFilter
```

### 드라이버 상태 확인

```cmd
fltmc instances
```

### 드라이버 필터 정보 확인

```cmd
fltmc filters
```

## 제거 방법 (Uninstallation)

1. 드라이버 중지:
   ```cmd
   fltmc unload MiniFilter
   ```

2. 드라이버 제거:
   ```cmd
   rundll32.exe setupapi,InstallHinfSection DefaultUninstall 132 MiniFilter.inf
   ```

## 디버깅 (Debugging)

드라이버의 디버그 출력을 보려면 DebugView 또는 WinDbg를 사용할 수 있습니다:

1. [DebugView](https://docs.microsoft.com/en-us/sysinternals/downloads/debugview) 다운로드
2. 관리자 권한으로 실행
3. Capture > Capture Kernel 활성화
4. 드라이버 작업 시 디버그 메시지 확인

## 프로젝트 구조 (Project Structure)

```
MiniFilter/
├── MiniFilter.c         # 메인 드라이버 소스 코드
├── MiniFilter.h         # 헤더 파일
├── MiniFilter.inf       # 드라이버 설치 정보 파일
├── MiniFilter.vcxproj   # Visual Studio 프로젝트 파일
├── MiniFilter.sln       # Visual Studio 솔루션 파일
├── .gitignore          # Git 제외 파일 목록
└── README.md           # 이 파일
```

## 주요 콜백 함수 (Key Callback Functions)

- `MiniFilterPreCreate` / `MiniFilterPostCreate`: 파일 생성 작업 처리
- `MiniFilterPreRead` / `MiniFilterPostRead`: 파일 읽기 작업 처리
- `MiniFilterPreWrite` / `MiniFilterPostWrite`: 파일 쓰기 작업 처리

## 보안 고려사항 (Security Considerations)

- 이 드라이버는 커널 모드에서 실행되므로 신중하게 개발되어야 합니다
- 프로덕션 환경에서는 반드시 코드 서명이 필요합니다
- 테스트는 가상 머신에서 수행하는 것을 권장합니다

## 라이선스 (License)

이 프로젝트는 교육 및 참고 목적으로 제공됩니다.

## 참고 자료 (References)

- [Windows Driver Kit (WDK)](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
- [File System Minifilter Drivers](https://docs.microsoft.com/en-us/windows-hardware/drivers/ifs/filter-manager-concepts)
- [Filter Manager](https://docs.microsoft.com/en-us/windows-hardware/drivers/ifs/filter-manager-and-minifilter-driver-architecture)
