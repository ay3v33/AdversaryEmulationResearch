# Windows AV bypass method using Discord
## DISCLAIMER: FOR EDUCATIONAL PURPOSES ONLY
This project, including all source code and documentation, is intended strictly for educational and research purposes related to cybersecurity and Windows internals. The author (Aidan Van Voorhis) does not condone, encourage, or support the use of this software for unauthorized access, data theft, or any other illegal activity.

The code is provided "AS IS" without warranty of any kind. In no event shall the author be liable for any claim, damages, or other liability arising from the use of this software. By using this code, you agree that you are solely responsible for your actions and comply with all applicable local and international laws.

## How it works
### How Windows EDR/AV works
Normally, when a program calls a Windows API Function(like GetAsyncKeyState), which eventually transitions through ntdll.dll to reach the kernel. Security software "hooks" these functions, placing a jmp instruction at the start of the code that runs the anti-malware detection code before running the actual function. To avoid this "hooking" I used manual indirect syscalls. 

## The Manual Syscall
This is the specific function that is bypassing the hook:
Parameters are (key, keySSN, trustedGadget)
DirectSyscallBridge PROC
    mov r10, rcx
    mov eax, edx
    jmp r8
DirectSyscallBridge ENDP

The mov r10, rcx instruction is vital because the syscall instruction automatically overwrites the rcx register with the return address. Moving the first parameter to r10 preserves the data so the Kernel can read it correctly.
Moving edx into eax is telling the cpu I want to run the function that the keySSN parameter is holding.
The jmp R8 instruction is the code 'jumping' to the code in a trusted dll, which then makes the legitmate syscall, but since the GetAsyncKeyState ssn was just loaded into eax it runs that code in the kernel instead.

## How to find a syscall in a trusted dll
The trusted windows dll that I used was win32u.dll. The FindSysCallGadget function performs a hex search through the DLL's memory looking for 0x0F 0x05 (the assembly code for syscall). Once found it returns the memory address(an address located in a trusted section of memory) of those bytes. 

## Finding the syscall ID for the NtUserGetAsyncKeyState function
In a perfect world (with no AV enabled) the NtUserGetAsyncKeyState function inside win32u.dll will alway start with the same pattern of bytes. However, when an AV is active, it modifies the DLL in memory. It overwrites the first few bytes with a JMP instruction (0xE9) to send the code to the AV scanner. If the malware tries to read the ID from a hooked function, it will see the address of the AV's scanner instead of the SSN. This is where the 'Halos gate' logic comes into play. Because windows syscall IDs are usually sequential, the syscall ID can be calculated by looking at the IDs of the functions immeadiately above and below it.

## DLL Side loading: The injection method
Instead of trying to inject code into a running process, which is highly monitored, this malware poses as a legitimate system file. When injecting the code, I rename the file dxgi.dll, which is a commonly used DirectX Graphics Infrastructure library used by many Windows Apps including Discord. When discord starts it looks for the real dxgi.dll but by placing the dll in the discord folder it loads the malware instead. To ensure discord can still use the real dxgi functions, we put lines that look like this
`#pragma comment(linker, "/export:CreateDXGIFactory=C:\\Windows\\System32\\dxgi.CreateDXGIFactory")`
at the top of the dllmain.cpp file to tell Discord where to find the real code so that it doesn't crash.

## Data Exfiltration
The keystrokes are captured and stored in a RollingSecureBuffer class, so that the data is only stored while encrypted in RAM. Once the buffer hits a certain size, the data is exfiltrated using a discord webhook and is immediately wiped from the buffer. Using a discord webhook is perfect for this type of attack because the malware is already running inside of Discord.exe making data sent to Discord.com appear perfectly normal. Additionally, the webhook URL and Token are not stored as plain text, but are obfuscated, preventing static analysis tools from simply searching the file for URLs.
