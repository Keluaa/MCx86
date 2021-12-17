
#include <iostream>
#include <iomanip>
#include <string>
#include <map>

typedef unsigned int U32;
static_assert(sizeof(U32) == 4);


constexpr U32 CF	= 1 << 0;
constexpr U32 PF	= 1 << 2;
constexpr U32 AF	= 1 << 4;
constexpr U32 ZF	= 1 << 6;
constexpr U32 SF	= 1 << 7;
constexpr U32 TF	= 1 << 8;
constexpr U32 IF	= 1 << 9;
constexpr U32 DF	= 1 << 10;
constexpr U32 OF	= 1 << 11;
constexpr U32 IOPL	= 0b11 << 12;
constexpr U32 IOPL_L= 1 << 12;
constexpr U32 IOPL_H= 0b10 << 12;
constexpr U32 NT	= 1 << 14;
constexpr U32 RF	= 1 << 16;
constexpr U32 VM	= 1 << 17;
constexpr U32 AC	= 1 << 18;
constexpr U32 VIF	= 1 << 19;
constexpr U32 VIP	= 1 << 20;
constexpr U32 ID	= 1 << 21;


std::string print_flags(U32 value)
{
    static const std::map<U32, const char*> flags_map{
            { CF, "CF" },
            { PF, "PF" },
            { AF, "AF" },
            { ZF, "ZF" },
            { SF, "SF" },
            { TF, "TF" },
            //{ IF, "IF" }, ignored because always present
            { DF, "DF" },
            { OF, "OF" },
            { IOPL, "IOPL" },
            { IOPL_L, "IOPL_L" },
            { IOPL_H, "IOPL_H" },
            { NT, "NT" },
            { RF, "RF" },
            { VM, "VM" },
            { AC, "AC" },
            { VIF, "VIF" },
            { VIP, "VIP" },
            { ID, "ID" },
    };

    std::string str("[");

    for (const auto& [ flag, flag_str ] : flags_map ) {
        if (value & flag) {
            str += " ";
            str += flag_str;
        }
    }

    str += " ]";
    return str;
}

// compile with '-m32 -masm=intel -std=c++20' using g++ 10+

int main()
{
    int flags = 0;
    int res = 0;
    int a = 0x2;
    int b = 0x7;

    __asm volatile(
        "\npush 0" // Clear EFlags
        "\npopfd"
        "\nmov %%ecx, %2"
        "\nsub %%ecx, %3"
        "\npushfd" // Get EFlags
        "\npop %0"
        "\nmov %1, %%ecx"
        : "=rm" (flags), "=rm" (res)
        : "m" (a), "m" (b)
    );

    std::cout << std::hex;
    std::cout << "a - b = 0x" << a << " - 0x" << b << "\n";
    std::cout << "Res: 0x" << res << "\n";
    std::cout << "Flags: " << print_flags(flags) << "\n";
}

