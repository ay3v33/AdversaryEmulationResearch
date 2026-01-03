# Windows AV bypass method using Discord
## DISCLAIMER: FOR EDUCATIONAL PURPOSES ONLY
This project, including all source code and documentation, is intended strictly for educational and research purposes related to cybersecurity and Windows internals. The author (Aidan Van Voorhis) does not condone, encourage, or support the use of this software for unauthorized access, data theft, or any other illegal activity.

The code is provided "AS IS" without warranty of any kind. In no event shall the author be liable for any claim, damages, or other liability arising from the use of this software. By using this code, you agree that you are solely responsible for your actions and comply with all applicable local and international laws.

## Project Goal
As a computer science student I have become increasingly interested in cybersecurity and the idea of hacking as seemed like magic to me. This has led me to put effort into understanding how practical hacks, that I have seen used maliciously against others, are created and more importantly how to defend against them, and potentially even use them to gain knowledge against the attacker. Many of the cybersecurity lessons I have gone through are extremely impractical and it makes it much harder to learn. I ultimately want to write this in a way that professionals and curious students like myself can understand.

## How it works
### How Windows EDR/AV works
Normally, when a program calls a Windows API (like GetAsyncKeyState), it calls a function in a DLL (like user32.dll). Security software "hooks" these functions, placing a jmp instruction at the start of the code that runs the anti-malware detection code before running the actual function. To avoid this "hooking" I used a manual syscall. 

## The Manual Syscall
This is the specific function that is bypassing the hook:
Parameters are (key, keySSN, trustedGadget)
DirectSyscallBridge PROC
    mov r10, rcx         ; 1st argument to R10
    mov eax, edx
    jmp r8
DirectSyscallBridge ENDP

Moving edx into eax is telling the cpu I want to run the function that the keySSN parameter is holding.
The jmp R8 instruction is the code 'jumping' to the code in a trusted dll, which then makes the legitmate syscall, but since the GetAsyncKeyState ssn was just loaded into eax it runs that code in the kernel instead.

## How to find a syscall in a trusted dll
The trusted windows dll that I used was win32u.dll. The FindSysCallGadget function performs a hex search through the DLL's memory looking for 0x0F 0x05 (the assembly code for syscall). Once found it returns the memory address(an address located in a trusted section of memory) of those bytes. 

## Finding the syscall ID for the NtUserGetAsyncKeyState function
In a perfect world (with no AV enabled) the NtUserGetAsyncKeyState function inside win32u.dll will alway start with the same pattern of bytes. However, when an AV is active, it modifies the DLL in memory. It overwrites the first few bytes with a JMP instruction (0xE9) to send the code to the AV scanner. If the malware tries to read the ID from a hooked function, it will see the address of the AV's scanner instead of the SSN. This is where the 'Hells gate' logic comes into play. Because windows syscall IDs are usually sequential, the syscall ID can be calculated by looking at the IDs of the functions immeadiately above and below it.

