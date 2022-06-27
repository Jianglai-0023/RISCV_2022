#include <iostream>
#include<cstdio>
using namespace std;
//#ifndef RISC_V_CPU_HPP
//#define RISC_V_CPU_HPP

//#include"wrapper.hpp"
#include<bitset>
#include <cstring>
#include<iomanip>
using namespace std;
using u32 = unsigned int;
using u8 = unsigned char;
class cpu {
    u8 Memory[1048576];
    u32 Register[32];
    u32 PC;
    u32 clk=0;
    string op[37]={
            "LUI",    // U    Load Upper Immediate
            "AUIPC",  // U    Add Upper Immediate to PC
            "JAL",    // UJ   Jump & Link
            "JALR",   // I    Jump & Link Register
            "BEQ",    // SB   Branch Equal
            "BNE",    // SB   Branch Not Equal
            "BLT",    // SB   Branch Less Than
            "BGE",    // SB   Branch Greater than or Equal
            "BLTU",   // SB   Branch Less than Unsigned
            "BGEU",   // SB   Branch Greater than or Equal Unsigned
            "LB",     // I    Load Byte
            "LH",     // I    Load Halfword
            "LW",     // I    Load Word
            "LBU",    // I    Load Byte Unsigned
            "LHU",    // I    Load Halfword Unsigned
            "SB",     // S    Store Byte
            "SH",     // S    Store Halfword
            "SW",     // S    Store Word
            "ADDI",   // I    ADD Immediate
            "SLTI",   // I    Set Less than Immediate
            "SLTIU",  // I    Set Less than Immediate Unsigned
            "XORI",   // I    XOR Immediate
            "ORI",    // I    OR Immediate
            "ANDI",   // I    AND Immediate
            "SLLI",   // I    Shift Left Immediate
            "SRLI",   // I    Shift Right Immediate
            "SRAI",   // I    Shift Right Arith Immediate
            "ADD",    // R    ADD
            "SUB",    // R    SUBtract
            "SLL",    // R    Shift Left
            "SLT",    // R    Set Less than
            "SLTU",   // R    Set Less than Unsigned
            "XOR",    // R    XOR
            "SRL",    // R    Shift Right
            "SRA",    // R    Shift Right Arithmetic
            "OR",     // R    OR
            "AND"     // R    AND
    };
    enum OPT {
        LUI,    // U    Load Upper Immediate
        AUIPC,  // U    Add Upper Immediate to PC
        JAL,    // UJ   Jump & Link
        JALR,   // I    Jump & Link Register
        BEQ,    // SB   Branch Equal
        BNE,    // SB   Branch Not Equal
        BLT,    // SB   Branch Less Than
        BGE,    // SB   Branch Greater than or Equal
        BLTU,   // SB   Branch Less than Unsigned
        BGEU,   // SB   Branch Greater than or Equal Unsigned
        LB,     // I    Load Byte
        LH,     // I    Load Halfword
        LW,     // I    Load Word
        LBU,    // I    Load Byte Unsigned
        LHU,    // I    Load Halfword Unsigned
        SB,     // S    Store Byte
        SH,     // S    Store Halfword
        SW,     // S    Store Word
        ADDI,   // I    ADD Immediate
        SLTI,   // I    Set Less than Immediate
        SLTIU,  // I    Set Less than Immediate Unsigned
        XORI,   // I    XOR Immediate
        ORI,    // I    OR Immediate
        ANDI,   // I    AND Immediate
        SLLI,   // I    Shift Left Immediate
        SRLI,   // I    Shift Right Immediate
        SRAI,   // I    Shift Right Arith Immediate
        ADD,    // R    ADD
        SUB,    // R    SUBtract
        SLL,    // R    Shift Left
        SLT,    // R    Set Less than
        SLTU,   // R    Set Less than Unsigned
        XOR,    // R    XOR
        SRL,    // R    Shift Right
        SRA,    // R    Shift Right Arithmetic
        OR,     // R    OR
        AND     // R    AND
    } opt;
    struct IDResult{
        OPT opt;
        u32 imm;
        u8 rd,rs1,rs2;
    };
    IDResult Rtype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rs1=gr(19,15,so);
        ans.rs2=gr(24,20,so);
        ans.rd=gr(11,7,so);
        return ans;
    }
    IDResult Utype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.imm= gr(31,12,so)<<12;
        ans.rd= gr(11,7,so);
        return ans;
    }
    IDResult Jtype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
//        cout <<bitset<sizeof(so)*8>(so) <<" SO"<<endl;
        ans.imm  = (so & 0b10000000'00000000'00000000'00000000u) >> 11u;   // 20
        ans.imm |= (so & 0b01111111'11100000'00000000'00000000u) >> 20u;   // 10:1
        ans.imm |= (so & 0b00000000'00010000'00000000'00000000u) >> 9u;    // 11
        ans.imm |= (so & 0b00000000'00001111'11110000'00000000u);          // 19:12
        ans.imm=se(ans.imm,20);
//        cout <<bitset<sizeof(so)*8>(so) <<" SO"<< endl;
//        cout <<setbase(16)<<so <<" SO"<< endl;
//        cout <<bitset<sizeof(gr(30,21,so))*8>(gr(30,21,so)) <<" &"<< endl;
//        ans.imm= se((gr(31,31,so)<<20)+ (gr(30,21,so)<<1)+ (gr(20,20,so)<<11)+ (gr(19,12,so)<<12),20);
//        cout <<bitset<sizeof(gr(30,21,so))*8>(gr(30,21,so)) <<" First"<< endl;



//        cout <<bitset<sizeof(ans.imm)*8>(ans.imm) <<" IMM"<< endl;
//        cout << ans.imm << endl;
//        exit(0);
        ans.rd=gr(11,7,so);
        return ans;
    }
    IDResult Itype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rd= gr(11,7,so);
        ans.imm= se(gr(31,20,so),11);
        ans.rs1= gr(19,15,so);
        return ans;
    }
    IDResult Stype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rs1=gr(19,15,so);
        ans.rs2=gr(24,20,so);
//        ans.rd=gr(11,7,so);
        ans.imm=se((gr(31,25,so)<<5)+gr(11,7,so),11);
        return ans;
    }
    IDResult Btype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rs1= gr(19,15,so);
        ans.rs2=gr(24,20,so);
        ans.imm= se((gr(31,31,so)<<12)+(gr(30,25,so)<<5)+(gr(11,8,so)<<1)+(gr(7,7,so)<<11),12);
        return ans;
    }
    inline u32 gr(int l, int r, u32 so) {//0-base
        u32 ans = (so<<(31-l));
        ans=ans>>(r+(31-l));
        return ans;
    }

    void parser(std::istream &Input) {
        u32 point=0;
        while (!Input.fail()) {
            std::string s;
            Input >> s;
            if (s[0] == '@') {
                char *p;
                point = strtoul(s.substr(1, 8).c_str(), &p, 16);
            } else {
                char *p;
                Memory[point] = strtoul(s.c_str(), &p, 16);
                point++;
            }
        }
    }
    u32 se(u32 imm,u32 index){//index为非零下标 0-based
        if(!(imm&(1u<<index)))return imm&(0b11111111'11111111'11111111'11111111u >> (31-index));//if 符号位是0
        else{
            return imm|(0b11111111'11111111'11111111'11111111u << (index + 1));
        }
    }

    u32 IF() {
        u32 point = PC;
        u32 so=u32(Memory[point])+(u32(Memory[point + 1])<<8)+(u32(Memory[point + 2])<<16)+(u32(Memory[point + 3])<<24);
        return so;
    }

    IDResult ID(u32 so) {
        IDResult ans;
        u32 optcode = gr(6, 0, so);
        u32 OPT3 = gr(14, 12, so);
        u32 OPT4 = gr(31, 25, so);
        switch (optcode) {
            case 0b0110111:
                ans= Utype(so,LUI);
                break;
            case 0b0010111:
                ans= Utype(so,AUIPC);
                break;
            case 0b1101111:
                ans= Jtype(so,JAL);
                break;
            case 0b1100111:
                ans=Itype(so,JALR);
                break;
            case 0b1100011://B
                switch (OPT3) {
                    case 0b000:
                        ans= Btype(so,BEQ);
                        break;
                    case 0b001:
                        ans= Btype(so,BNE);
                        break;
                    case 0b100:
                        ans= Btype(so,BLT);
                        break;
                    case 0b101:
                        ans=Btype(so,BGE);
                        break;
                    case 0b110:
                        ans= Btype(so,BLTU);
                        break;
                    case 0b111:
                        ans= Btype(so,BGEU);
                        break;
                    default:{
                        throw("case 0b1100011 DC");
                    }
                }break;
            case 0b0000011://I
                switch (OPT3) {
                    case 0b000:
                        ans= Itype(so,LB);
                        break;
                    case 0b001:
                        ans= Itype(so,LH);
                        break;
                    case 0b010:
                        ans= Itype(so,LW);
                        break;
                    case 0b100:
                        ans= Itype(so,LBU);
                        break;
                    case 101:
                        ans= Itype(so,LHU);
                        break;
                    default:throw("case 0b0000011 DC!");
                }break;
            case 0b0100011://S
                switch (OPT3) {
                    case 0b000:
                        ans= Stype(so,SB);
                        break;
                    case 0b001:
                        ans= Stype(so,SH);
                        break;
                    case 0b010:
                        ans= Stype(so,SW);
                        break;
                    default: throw("line210 OPT3 NOT reach!");
                }
                break;
            case 0b0010011://I
                switch (OPT3) {
                    case 0b000:
                        ans= Itype(so,ADDI);
                        break;
                    case 0b010:
                        ans= Itype(so,SLTI);
                        break;
                    case 0b011:
                        ans= Itype(so,SLTIU);
                        break;
                    case 0b100:
                        ans= Itype(so,XORI);break;
                    case 0b110:
                        ans= Itype(so,ORI);break;
                    case 0b111:
                        ans= Itype(so,ANDI);break;
                    case 0b001:
                        ans.opt = SLLI;
                        ans.rs1=gr(19,15,so);
                        ans.rd=gr(11,7,so);
                        ans.imm=gr(24,20,so);break;
                    case 0b101:
                        switch (OPT4) {
                            case 0b0000000:
                                ans.opt = SRLI;
                                ans.rs1=gr(19,15,so);
                                ans.rd=gr(11,7,so);
                                ans.imm=gr(24,20,so);break;
                            case 0b0100000:
                                ans.opt = SRAI;
                                ans.rs1=gr(19,15,so);
                                ans.rd=gr(11,7,so);
                                ans.imm=gr(24,20,so);break;
                            default:throw("line246 OPT3 not reach!");
                        }
                        break;
                }
                break;
            case 0b0110011:
                switch (OPT3) {
                    case 0b000:
                        switch (OPT4) {
                            case 0b0000000:
                                ans=Rtype(so,ADD);break;
                            case 0b0100000:
                                ans=Rtype(so,SUB);break;
                            default: throw("line313 OPT4 not reach!");
                        }
                        break;
                    case 0b001:
                        ans=Rtype(so,SLL);break;
                    case 0b010:
                        ans=Rtype(so,SLT);break;
                    case 0b011:
                        ans=Rtype(so,SLTU);break;
                    case 0b100:
                        ans= Rtype(so,XOR);break;
                    case 0b101:
                        switch (OPT4) {
                            case 0b0000000:
                                ans=Rtype(so,SRL);break;
                            case 0b0100000:
                                ans=Rtype(so,SRA);break;
                            default: throw("line274 OPT4 not reach!");
                        }break;
                    case 0b110:
                        ans=Rtype(so,OR);break;
                    case 0b111:
                        ans= Rtype(so,AND);break;
                    default:throw("line 280 OPT3 not reach!");
                }
                break;
            default:throw("line 282 OPT not reach!");
        }
        return ans;
    }
    struct Mem{
        bool used=false;
        bool isload;
        u32 md;
        u8 imm;
    };
    struct Res{
        bool iswrite;
        u8 rd;
        u32 imm;
    };
    struct EXResult{
        Mem mem;
        Res res;
    };

    void EX(IDResult p,bool &halt,bool &jump){//p->pack
        u32 rd=p.rd;
        Register[0] = 0;
        u32 rs1=Register[u32(p.rs1)];
        u32 rs2=Register[u32(p.rs2)];
        u32 imm=p.imm;
        u32 pc=PC;
        switch(p.opt){
            case LB:
                Register[rd]=se(u32(Memory[rs1+imm]),7);
                break;
            case LH:
                Register[rd]=se(u32(Memory[rs1+imm])+(u32(Memory[rs1+imm+1])<<8),15);
                break;
            case LW:
                Register[rd]=u32(Memory[rs1+imm])+(u32(Memory[rs1+imm+1])<<8)+(u32(Memory[rs1+imm+2])<<16)+(u32(Memory[rs1+imm+3])<<24);
                break;
            case LBU:
                Register[rd]=u32(Memory[rs1+imm]);
                break;
            case LHU:
                Register[rd]=u32(Memory[rs1+imm])+(u32(Memory[rs1+imm+1])<<8);
                break;
            case SB:
                Memory[rs1+imm]=u8(rs2);
                break;
            case SH:
                Memory[rs1+imm]=u8(rs2);
                Memory[rs1+imm+1]=u8(rs2>>8);
                break;
            case SW:
                Memory[rs1+imm]=rs2&0xff;
                Memory[rs1+imm+1]=(rs2>>8)&0xff;
                Memory[rs1+imm+2]=(rs2>>16)&0xff;
                Memory[rs1+imm+3]=(rs2>>24)&0xff;
                break;
            case ADD:
                Register[rd]=rs1+rs2;
                break;
            case ADDI:
                if(rd==10&&imm==255){
                    halt=true;
                    break;
                }
                Register[rd]=rs1+imm;
                break;
            case SUB:
                Register[rd]=rs1-rs2;
                break;
            case LUI:
                Register[rd]=imm;
                break;
            case AUIPC:
                Register[rd]=pc+imm;
                break;
            case XOR:
                Register[rd]=rs1^rs2;
                break;
            case XORI:
                Register[rd]=rs1^imm;
                break;
            case OR:
                Register[rd]=rs1|rs2;
                break;
            case ORI:
                Register[rd]=rs1=rs1|imm;
                break;
            case AND:
                Register[rd]=rs1&rs2;
                break;
            case ANDI:
                Register[rd]=rs1&imm;
                break;
            case SLL:
                Register[rd]=rs1<<rs2;
                break;
                //todo 低五位有效
            case SLLI:
                Register[rd]=rs1<<imm;
                break;
            case SRL:
                Register[rd]=rs1>>rs2;
                break;
            case SRLI:
                Register[rd]=rs1>>imm;
                break;
            case SRA:
                Register[rd]=se(rs1>>rs2,31-rs2);
                break;
            case SRAI:
                Register[rd]=se(rs1>>imm,31-imm);
                break;
            case SLT:
                Register[rd]=(int(rs1)<int(rs2));
                break;
            case SLTI:
                Register[rd]=(int(rs1)<int(imm));
                break;
            case SLTU:
                Register[rd]=(rs1<rs2);
                break;
            case SLTIU:
                Register[rd]=(rs1<imm);
                break;
            case BEQ:
                if(rs1==rs2){//116c
                    pc+=imm;
                    jump=true;
                }
                break;
            case BNE:
                if(rs1!=rs2){
                    pc+=imm;
                    jump=true;
                }
                break;
            case BLT:
                if(int(rs1)<int(rs2)){
                    pc+=imm;
                    jump= true;
                }
                break;
            case BGE:
                if(int(rs1)>=int(rs2)){
                    jump=true;
                    pc+=imm;
                }
                break;
            case BLTU:
                if(rs1<rs2){
                    pc+=imm;
                    jump=true;
                }
                break;
            case BGEU:
                if(rs1>=rs2){
                    jump=true;
                    pc+=imm;
                }
                break;
            case JAL:
                Register[rd]=pc+4;
                pc+=imm;
                jump=true;
                break;
            case JALR:
                Register[rd]=pc+4;
                pc=(rs1+imm)&(~1);
                jump=true;
                break;

            default:throw("no case excute!");
        }
        PC=pc;
    }
public:
    void init(std::istream &Input) {
        memset(Memory,0,sizeof(Memory));
        parser(Input);
        memset(Register,0,sizeof(Register));
        PC = 0;
    }
    u32 run() {
        bool flag = false;
        while(!flag){
            bool jump=false;
            u32 so=IF();
            IDResult pack = ID(so);
            EX(pack,flag,jump);
            if(!jump)PC+=4;
            ++clk;
        }
        return Register[10];
    }
};
//#endif //RISC_V_CPU_HPP
int main() {

    freopen("data.in","r",stdin);
    freopen("data.out","w",stdout);
    u32 ans;
    cpu risc_v;
    try{
        risc_v.init(cin);
        ans=risc_v.run();
    }
    catch(const char *d){
        cerr << d << endl;
    }
    ans&=255u;
    cout << ans << endl;
}
#include <iostream>
#include<cstdio>
using namespace std;
//#ifndef RISC_V_CPU_HPP
//#define RISC_V_CPU_HPP

//#include"wrapper.hpp"
#include<bitset>
#include <cstring>
#include<iomanip>
using namespace std;
using u32 = unsigned int;
using u8 = unsigned char;
class cpu {
    u8 Memory[1048576];
    u32 Register[32];
    u32 PC;
    u32 clk=0;
    string op[37]={
            "LUI",    // U    Load Upper Immediate
            "AUIPC",  // U    Add Upper Immediate to PC
            "JAL",    // UJ   Jump & Link
            "JALR",   // I    Jump & Link Register
            "BEQ",    // SB   Branch Equal
            "BNE",    // SB   Branch Not Equal
            "BLT",    // SB   Branch Less Than
            "BGE",    // SB   Branch Greater than or Equal
            "BLTU",   // SB   Branch Less than Unsigned
            "BGEU",   // SB   Branch Greater than or Equal Unsigned
            "LB",     // I    Load Byte
            "LH",     // I    Load Halfword
            "LW",     // I    Load Word
            "LBU",    // I    Load Byte Unsigned
            "LHU",    // I    Load Halfword Unsigned
            "SB",     // S    Store Byte
            "SH",     // S    Store Halfword
            "SW",     // S    Store Word
            "ADDI",   // I    ADD Immediate
            "SLTI",   // I    Set Less than Immediate
            "SLTIU",  // I    Set Less than Immediate Unsigned
            "XORI",   // I    XOR Immediate
            "ORI",    // I    OR Immediate
            "ANDI",   // I    AND Immediate
            "SLLI",   // I    Shift Left Immediate
            "SRLI",   // I    Shift Right Immediate
            "SRAI",   // I    Shift Right Arith Immediate
            "ADD",    // R    ADD
            "SUB",    // R    SUBtract
            "SLL",    // R    Shift Left
            "SLT",    // R    Set Less than
            "SLTU",   // R    Set Less than Unsigned
            "XOR",    // R    XOR
            "SRL",    // R    Shift Right
            "SRA",    // R    Shift Right Arithmetic
            "OR",     // R    OR
            "AND"     // R    AND
    };
    enum OPT {
        LUI,    // U    Load Upper Immediate
        AUIPC,  // U    Add Upper Immediate to PC
        JAL,    // UJ   Jump & Link
        JALR,   // I    Jump & Link Register
        BEQ,    // SB   Branch Equal
        BNE,    // SB   Branch Not Equal
        BLT,    // SB   Branch Less Than
        BGE,    // SB   Branch Greater than or Equal
        BLTU,   // SB   Branch Less than Unsigned
        BGEU,   // SB   Branch Greater than or Equal Unsigned
        LB,     // I    Load Byte
        LH,     // I    Load Halfword
        LW,     // I    Load Word
        LBU,    // I    Load Byte Unsigned
        LHU,    // I    Load Halfword Unsigned
        SB,     // S    Store Byte
        SH,     // S    Store Halfword
        SW,     // S    Store Word
        ADDI,   // I    ADD Immediate
        SLTI,   // I    Set Less than Immediate
        SLTIU,  // I    Set Less than Immediate Unsigned
        XORI,   // I    XOR Immediate
        ORI,    // I    OR Immediate
        ANDI,   // I    AND Immediate
        SLLI,   // I    Shift Left Immediate
        SRLI,   // I    Shift Right Immediate
        SRAI,   // I    Shift Right Arith Immediate
        ADD,    // R    ADD
        SUB,    // R    SUBtract
        SLL,    // R    Shift Left
        SLT,    // R    Set Less than
        SLTU,   // R    Set Less than Unsigned
        XOR,    // R    XOR
        SRL,    // R    Shift Right
        SRA,    // R    Shift Right Arithmetic
        OR,     // R    OR
        AND     // R    AND
    } opt;
    struct IDResult{
        OPT opt;
        u32 imm;
        u8 rd,rs1,rs2;
    };
    IDResult Rtype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rs1=gr(19,15,so);
        ans.rs2=gr(24,20,so);
        ans.rd=gr(11,7,so);
        return ans;
    }
    IDResult Utype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.imm= gr(31,12,so)<<12;
        ans.rd= gr(11,7,so);
        return ans;
    }
    IDResult Jtype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
//        cout <<bitset<sizeof(so)*8>(so) <<" SO"<<endl;
        ans.imm  = (so & 0b10000000'00000000'00000000'00000000u) >> 11u;   // 20
        ans.imm |= (so & 0b01111111'11100000'00000000'00000000u) >> 20u;   // 10:1
        ans.imm |= (so & 0b00000000'00010000'00000000'00000000u) >> 9u;    // 11
        ans.imm |= (so & 0b00000000'00001111'11110000'00000000u);          // 19:12
        ans.imm=se(ans.imm,20);
//        cout <<bitset<sizeof(so)*8>(so) <<" SO"<< endl;
//        cout <<setbase(16)<<so <<" SO"<< endl;
//        cout <<bitset<sizeof(gr(30,21,so))*8>(gr(30,21,so)) <<" &"<< endl;
//        ans.imm= se((gr(31,31,so)<<20)+ (gr(30,21,so)<<1)+ (gr(20,20,so)<<11)+ (gr(19,12,so)<<12),20);
//        cout <<bitset<sizeof(gr(30,21,so))*8>(gr(30,21,so)) <<" First"<< endl;



//        cout <<bitset<sizeof(ans.imm)*8>(ans.imm) <<" IMM"<< endl;
//        cout << ans.imm << endl;
//        exit(0);
        ans.rd=gr(11,7,so);
        return ans;
    }
    IDResult Itype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rd= gr(11,7,so);
        ans.imm= se(gr(31,20,so),11);
        ans.rs1= gr(19,15,so);
        return ans;
    }
    IDResult Stype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rs1=gr(19,15,so);
        ans.rs2=gr(24,20,so);
//        ans.rd=gr(11,7,so);
        ans.imm=se((gr(31,25,so)<<5)+gr(11,7,so),11);
        return ans;
    }
    IDResult Btype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rs1= gr(19,15,so);
        ans.rs2=gr(24,20,so);
        ans.imm= se((gr(31,31,so)<<12)+(gr(30,25,so)<<5)+(gr(11,8,so)<<1)+(gr(7,7,so)<<11),12);
        return ans;
    }
    inline u32 gr(int l, int r, u32 so) {//0-base
        u32 ans = (so<<(31-l));
        ans=ans>>(r+(31-l));
        return ans;
    }

    void parser(std::istream &Input) {
        u32 point=0;
        while (!Input.fail()) {
            std::string s;
            Input >> s;
            if (s[0] == '@') {
                char *p;
                point = strtoul(s.substr(1, 8).c_str(), &p, 16);
            } else {
                char *p;
                Memory[point] = strtoul(s.c_str(), &p, 16);
                point++;
            }
        }
    }
    u32 se(u32 imm,u32 index){//index为非零下标 0-based
        if(!(imm&(1u<<index)))return imm&(0b11111111'11111111'11111111'11111111u >> (31-index));//if 符号位是0
        else{
            return imm|(0b11111111'11111111'11111111'11111111u << (index + 1));
        }
    }

    u32 IF() {
        u32 point = PC;
        u32 so=u32(Memory[point])+(u32(Memory[point + 1])<<8)+(u32(Memory[point + 2])<<16)+(u32(Memory[point + 3])<<24);
        return so;
    }

    IDResult ID(u32 so) {
        IDResult ans;
        u32 optcode = gr(6, 0, so);
        u32 OPT3 = gr(14, 12, so);
        u32 OPT4 = gr(31, 25, so);
        switch (optcode) {
            case 0b0110111:
                ans= Utype(so,LUI);
                break;
            case 0b0010111:
                ans= Utype(so,AUIPC);
                break;
            case 0b1101111:
                ans= Jtype(so,JAL);
                break;
            case 0b1100111:
                ans=Itype(so,JALR);
                break;
            case 0b1100011://B
                switch (OPT3) {
                    case 0b000:
                        ans= Btype(so,BEQ);
                        break;
                    case 0b001:
                        ans= Btype(so,BNE);
                        break;
                    case 0b100:
                        ans= Btype(so,BLT);
                        break;
                    case 0b101:
                        ans=Btype(so,BGE);
                        break;
                    case 0b110:
                        ans= Btype(so,BLTU);
                        break;
                    case 0b111:
                        ans= Btype(so,BGEU);
                        break;
                    default:{
                        throw("case 0b1100011 DC");
                    }
                }break;
            case 0b0000011://I
                switch (OPT3) {
                    case 0b000:
                        ans= Itype(so,LB);
                        break;
                    case 0b001:
                        ans= Itype(so,LH);
                        break;
                    case 0b010:
                        ans= Itype(so,LW);
                        break;
                    case 0b100:
                        ans= Itype(so,LBU);
                        break;
                    case 101:
                        ans= Itype(so,LHU);
                        break;
                    default:throw("case 0b0000011 DC!");
                }break;
            case 0b0100011://S
                switch (OPT3) {
                    case 0b000:
                        ans= Stype(so,SB);
                        break;
                    case 0b001:
                        ans= Stype(so,SH);
                        break;
                    case 0b010:
                        ans= Stype(so,SW);
                        break;
                    default: throw("line210 OPT3 NOT reach!");
                }
                break;
            case 0b0010011://I
                switch (OPT3) {
                    case 0b000:
                        ans= Itype(so,ADDI);
                        break;
                    case 0b010:
                        ans= Itype(so,SLTI);
                        break;
                    case 0b011:
                        ans= Itype(so,SLTIU);
                        break;
                    case 0b100:
                        ans= Itype(so,XORI);break;
                    case 0b110:
                        ans= Itype(so,ORI);break;
                    case 0b111:
                        ans= Itype(so,ANDI);break;
                    case 0b001:
                        ans.opt = SLLI;
                        ans.rs1=gr(19,15,so);
                        ans.rd=gr(11,7,so);
                        ans.imm=gr(24,20,so);break;
                    case 0b101:
                        switch (OPT4) {
                            case 0b0000000:
                                ans.opt = SRLI;
                                ans.rs1=gr(19,15,so);
                                ans.rd=gr(11,7,so);
                                ans.imm=gr(24,20,so);break;
                            case 0b0100000:
                                ans.opt = SRAI;
                                ans.rs1=gr(19,15,so);
                                ans.rd=gr(11,7,so);
                                ans.imm=gr(24,20,so);break;
                            default:throw("line246 OPT3 not reach!");
                        }
                        break;
                }
                break;
            case 0b0110011:
                switch (OPT3) {
                    case 0b000:
                        switch (OPT4) {
                            case 0b0000000:
                                ans=Rtype(so,ADD);break;
                            case 0b0100000:
                                ans=Rtype(so,SUB);break;
                            default: throw("line313 OPT4 not reach!");
                        }
                        break;
                    case 0b001:
                        ans=Rtype(so,SLL);break;
                    case 0b010:
                        ans=Rtype(so,SLT);break;
                    case 0b011:
                        ans=Rtype(so,SLTU);break;
                    case 0b100:
                        ans= Rtype(so,XOR);break;
                    case 0b101:
                        switch (OPT4) {
                            case 0b0000000:
                                ans=Rtype(so,SRL);break;
                            case 0b0100000:
                                ans=Rtype(so,SRA);break;
                            default: throw("line274 OPT4 not reach!");
                        }break;
                    case 0b110:
                        ans=Rtype(so,OR);break;
                    case 0b111:
                        ans= Rtype(so,AND);break;
                    default:throw("line 280 OPT3 not reach!");
                }
                break;
            default:throw("line 282 OPT not reach!");
        }
        return ans;
    }
    struct Mem{
        bool used=false;
        bool isload;
        u32 md;
        u8 imm;
    };
    struct Res{
        bool iswrite;
        u8 rd;
        u32 imm;
    };
    struct EXResult{
        Mem mem;
        Res res;
    };

    void EX(IDResult p,bool &halt,bool &jump){//p->pack
        u32 rd=p.rd;
        Register[0] = 0;
        u32 rs1=Register[u32(p.rs1)];
        u32 rs2=Register[u32(p.rs2)];
        u32 imm=p.imm;
        u32 pc=PC;
        switch(p.opt){
            case LB:
                Register[rd]=se(u32(Memory[rs1+imm]),7);
                break;
            case LH:
                Register[rd]=se(u32(Memory[rs1+imm])+(u32(Memory[rs1+imm+1])<<8),15);
                break;
            case LW:
                Register[rd]=u32(Memory[rs1+imm])+(u32(Memory[rs1+imm+1])<<8)+(u32(Memory[rs1+imm+2])<<16)+(u32(Memory[rs1+imm+3])<<24);
                break;
            case LBU:
                Register[rd]=u32(Memory[rs1+imm]);
                break;
            case LHU:
                Register[rd]=u32(Memory[rs1+imm])+(u32(Memory[rs1+imm+1])<<8);
                break;
            case SB:
                Memory[rs1+imm]=u8(rs2);
                break;
            case SH:
                Memory[rs1+imm]=u8(rs2);
                Memory[rs1+imm+1]=u8(rs2>>8);
                break;
            case SW:
                Memory[rs1+imm]=rs2&0xff;
                Memory[rs1+imm+1]=(rs2>>8)&0xff;
                Memory[rs1+imm+2]=(rs2>>16)&0xff;
                Memory[rs1+imm+3]=(rs2>>24)&0xff;
                break;
            case ADD:
                Register[rd]=rs1+rs2;
                break;
            case ADDI:
                if(rd==10&&imm==255){
                    halt=true;
                    break;
                }
                Register[rd]=rs1+imm;
                break;
            case SUB:
                Register[rd]=rs1-rs2;
                break;
            case LUI:
                Register[rd]=imm;
                break;
            case AUIPC:
                Register[rd]=pc+imm;
                break;
            case XOR:
                Register[rd]=rs1^rs2;
                break;
            case XORI:
                Register[rd]=rs1^imm;
                break;
            case OR:
                Register[rd]=rs1|rs2;
                break;
            case ORI:
                Register[rd]=rs1=rs1|imm;
                break;
            case AND:
                Register[rd]=rs1&rs2;
                break;
            case ANDI:
                Register[rd]=rs1&imm;
                break;
            case SLL:
                Register[rd]=rs1<<rs2;
                break;
                //todo 低五位有效
            case SLLI:
                Register[rd]=rs1<<imm;
                break;
            case SRL:
                Register[rd]=rs1>>rs2;
                break;
            case SRLI:
                Register[rd]=rs1>>imm;
                break;
            case SRA:
                Register[rd]=se(rs1>>rs2,31-rs2);
                break;
            case SRAI:
                Register[rd]=se(rs1>>imm,31-imm);
                break;
            case SLT:
                Register[rd]=(int(rs1)<int(rs2));
                break;
            case SLTI:
                Register[rd]=(int(rs1)<int(imm));
                break;
            case SLTU:
                Register[rd]=(rs1<rs2);
                break;
            case SLTIU:
                Register[rd]=(rs1<imm);
                break;
            case BEQ:
                if(rs1==rs2){//116c
                    pc+=imm;
                    jump=true;
                }
                break;
            case BNE:
                if(rs1!=rs2){
                    pc+=imm;
                    jump=true;
                }
                break;
            case BLT:
                if(int(rs1)<int(rs2)){
                    pc+=imm;
                    jump= true;
                }
                break;
            case BGE:
                if(int(rs1)>=int(rs2)){
                    jump=true;
                    pc+=imm;
                }
                break;
            case BLTU:
                if(rs1<rs2){
                    pc+=imm;
                    jump=true;
                }
                break;
            case BGEU:
                if(rs1>=rs2){
                    jump=true;
                    pc+=imm;
                }
                break;
            case JAL:
                Register[rd]=pc+4;
                pc+=imm;
                jump=true;
                break;
            case JALR:
                Register[rd]=pc+4;
                pc=(rs1+imm)&(~1);
                jump=true;
                break;

            default:throw("no case excute!");
        }
        PC=pc;
    }
public:
    void init(std::istream &Input) {
        memset(Memory,0,sizeof(Memory));
        parser(Input);
        memset(Register,0,sizeof(Register));
        PC = 0;
    }
    u32 run() {
        bool flag = false;
        while(!flag){
            bool jump=false;
            u32 so=IF();
            IDResult pack = ID(so);
            EX(pack,flag,jump);
            if(!jump)PC+=4;
            ++clk;
        }
        return Register[10];
    }
};
//#endif //RISC_V_CPU_HPP
int main() {

    freopen("data.in","r",stdin);
    freopen("data.out","w",stdout);
    u32 ans;
    cpu risc_v;
    try{
        risc_v.init(cin);
        ans=risc_v.run();
    }
    catch(const char *d){
        cerr << d << endl;
    }
    ans&=255u;
    cout << ans << endl;
}


