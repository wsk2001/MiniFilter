# MiniFilter

## 파일시스템 미니필터 (File System Mini-Filter Driver)

Windows 커널 모드 파일 시스템 미니필터 드라이버입니다. 이 드라이버는 파일 시스템 I/O 작업을 감시하고 로깅합니다.

A Windows kernel-mode file system mini-filter driver that monitors and logs file system I/O operations.

## Features (기능)

- **파일 생성/열기 모니터링** - File creation and open operations monitoring
- **읽기 작업 감시** - Read operation monitoring  
- **쓰기 작업 감시** - Write operation monitoring
- **실시간 로깅** - Real-time logging of file system activities
- **커널 모드 작동** - Operates in kernel mode for maximum performance
- **Filter Manager 통합** - Integrated with Windows Filter Manager

## System Requirements (시스템 요구사항)

- Windows 7 이상 / Windows 7 or later
- Windows Server 2008 R2 이상 / Windows Server 2008 R2 or later
- Windows 10/11 권장 / Windows 10/11 recommended
- 관리자 권한 필요 / Administrator privileges required
- x86, x64, ARM, ARM64 아키텍처 지원 / Support for x86, x64, ARM, ARM64 architectures

## Project Structure (프로젝트 구조)

```
MiniFilter/
├── src/                    # 소스 코드 / Source code
│   ├── MiniFilter.c       # 메인 드라이버 소스 / Main driver source
│   ├── MiniFilter.h       # 헤더 파일 / Header file
│   ├── MiniFilter.inf     # 설치 정보 파일 / Installation information
│   ├── MiniFilter.vcxproj # Visual Studio 프로젝트 / Visual Studio project
│   └── makefile           # 빌드 파일 / Build file
├── docs/                   # 문서 / Documentation
│   ├── INSTALLATION.md    # 설치 가이드 / Installation guide
│   └── DEVELOPMENT.md     # 개발 가이드 / Development guide
├── build/                  # 빌드 출력 / Build output (generated)
└── README.md              # 이 파일 / This file
```

## Quick Start (빠른 시작)

### Building (빌드)

1. **Windows Driver Kit (WDK) 설치 / Install Windows Driver Kit (WDK)**
   ```
   https://docs.microsoft.com/windows-hardware/drivers/download-the-wdk
   ```

2. **Visual Studio에서 빌드 / Build with Visual Studio**
   ```cmd
   # Open MiniFilter.vcxproj in Visual Studio
   # Select configuration (Debug/Release) and platform (x64)
   # Build solution (Ctrl+Shift+B)
   ```

3. **명령줄에서 빌드 / Build from command line**
   ```cmd
   cd src
   msbuild MiniFilter.vcxproj /p:Configuration=Release /p:Platform=x64
   ```

### Installation (설치)

1. **테스트 서명 모드 활성화 / Enable test signing mode**
   ```cmd
   bcdedit /set testsigning on
   # 재부팅 필요 / Reboot required
   ```

2. **드라이버 설치 / Install driver**
   ```cmd
   # Method 1: Right-click MiniFilter.inf and select "Install"
   
   # Method 2: Command line
   cd src
   rundll32.exe setupapi,InstallHinfSection DefaultInstall 132 .\MiniFilter.inf
   ```

3. **드라이버 로드 / Load driver**
   ```cmd
   fltmc load MiniFilter
   
   # 드라이버 확인 / Verify driver
   fltmc filters
   ```

### Usage (사용법)

**드라이버 시작 / Start driver**
```cmd
net start MiniFilter
# or
sc start MiniFilter
# or
fltmc load MiniFilter
```

**드라이버 중지 / Stop driver**
```cmd
net stop MiniFilter
# or
sc stop MiniFilter
# or
fltmc unload MiniFilter
```

**디버그 출력 보기 / View debug output**
- DebugView 사용 / Use DebugView: https://docs.microsoft.com/sysinternals/downloads/debugview
- WinDbg 사용 / Use WinDbg for kernel debugging

## Documentation (문서)

상세한 문서는 `docs/` 디렉토리를 참조하세요:

For detailed documentation, see the `docs/` directory:

- **[INSTALLATION.md](docs/INSTALLATION.md)** - 설치 및 설정 가이드 / Installation and configuration guide
- **[DEVELOPMENT.md](docs/DEVELOPMENT.md)** - 개발자 가이드 / Developer guide

## Architecture (아키텍처)

```
User Mode Applications
        ↓
I/O Manager (IRP)
        ↓
Filter Manager (FltMgr.sys)
        ↓
MiniFilter Driver (MiniFilter.sys) ← 이 프로젝트 / This project
        ↓
File System Driver (NTFS, FAT32, etc.)
        ↓
Storage Stack
```

## Monitored Operations (감시 작업)

현재 구현된 감시 작업:

Currently implemented monitoring operations:

- **IRP_MJ_CREATE** - 파일 생성/열기 / File create/open
- **IRP_MJ_READ** - 파일 읽기 / File read
- **IRP_MJ_WRITE** - 파일 쓰기 / File write

## Development (개발)

### Prerequisites (개발 필수 사항)

- Visual Studio 2019 이상 / Visual Studio 2019 or later
- Windows Driver Kit (WDK)
- Windows SDK
- 커널 모드 디버깅 기초 지식 / Basic knowledge of kernel mode debugging

### Building from Source (소스에서 빌드)

```cmd
# Clone repository
git clone https://github.com/wsk2001/MiniFilter.git
cd MiniFilter

# Build
cd src
msbuild MiniFilter.vcxproj /p:Configuration=Debug /p:Platform=x64
```

### Debugging (디버깅)

1. **커널 디버깅 설정 / Set up kernel debugging**
2. **DebugView로 로그 확인 / Monitor logs with DebugView**
3. **WinDbg로 심화 디버깅 / Advanced debugging with WinDbg**

자세한 내용은 [DEVELOPMENT.md](docs/DEVELOPMENT.md) 참조

See [DEVELOPMENT.md](docs/DEVELOPMENT.md) for details

## Safety and Security (안전 및 보안)

⚠️ **경고 / Warning**: 이 드라이버는 커널 모드에서 실행됩니다. 잘못된 사용은 시스템 불안정성이나 크래시를 일으킬 수 있습니다.

⚠️ **Warning**: This driver runs in kernel mode. Improper use may cause system instability or crashes.

**보안 권장사항 / Security Recommendations:**
- 프로덕션 환경에서는 코드 서명된 드라이버만 사용 / Only use code-signed drivers in production
- 테스트는 가상 머신에서 진행 / Test in virtual machines
- 정기적인 코드 감사 수행 / Perform regular code audits

## Contributing (기여)

기여는 환영합니다! 다음 절차를 따라주세요:

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License (라이선스)

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support (지원)

문제가 있거나 질문이 있으시면 GitHub Issues를 사용해주세요.

For issues or questions, please use GitHub Issues.

## Acknowledgments (감사의 말)

- Microsoft Windows Driver Kit (WDK) Documentation
- OSR Online Driver Development Community
- Windows Internals book series

## Resources (참고 자료)

- [Windows Driver Kit Documentation](https://docs.microsoft.com/windows-hardware/drivers/)
- [File System Minifilter Drivers](https://docs.microsoft.com/windows-hardware/drivers/ifs/file-system-minifilter-drivers)
- [Filter Manager Concepts](https://docs.microsoft.com/windows-hardware/drivers/ifs/filter-manager-concepts)
- [OSR Online](https://www.osronline.com/)

---

**Note**: This is a basic implementation suitable for learning and development. Production use requires additional error handling, security hardening, and proper code signing.
