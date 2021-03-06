#ifndef PPCA_TEST_TOMASULO_HPP
#define PPCA_TEST_TOMASULO_HPP
using u32 = unsigned int;
using u8 = unsigned char;
const int QUEUE_SIZE = 32;
const int REG_SIZE = 32;
const int RS_SIZE = 32;
class saturating_counter{
public:
    int predict[65535];
    const int M=65535;
public:
    bool jump_predict(int index0){
        int index=index0&M;
        if(predict[index]>=0&&predict[index]<2)return false;
        return true;
    }
    void result(int index0,bool jump){
        int index=index0&M;
        if(jump&&predict[index]<2)predict[index]++;
        else if(!jump&&predict[index]>0)predict[index]--;
    }
    saturating_counter(){
//        for(int i = 0; i < M; ++i)predict[i]=0;
        memset(predict,0,sizeof(predict));
    }
    ~saturating_counter(){};
}branch_predictor;
class simulator {
private:
    u8 Memory[1048576];
    u32 pre_register[32];
    u32 now_register[32];
    int pre_rename[32];
    int now_rename[32];
    int PC;
    int lsb_clk = 0;
    int clk = 0;
    bool issue_halt = false;
    bool jump_wrong=false;
    int change_pc_to=0;
    std::string PP[40] = {
            "NONE",
            "LB",     // I    Load Byte
            "LH",     // I    Load Halfword
            "LW",     // I    Load Word
            "LBU",    // I    Load Byte Unsigned
            "LHU",    // I    Load Halfword Unsigned
            "SB",     // S    Store Byte
            "SH",     // S    Store Halfword
            "SW",     // S    Store Word
            //----------------//
            "BEQ",    // SB   Branch Equal
            "BNE",    // SB   Branch Not Equal
            "BLT",    // SB   Branch Less Than
            "BGE",    // SB   Branch Greater than or Equal
            "BLTU",   // SB   Branch Less than Unsigned
            "BGEU",   // SB   Branch Greater than or Equal Unsigned
            "JAL",    // UJ   Jump & Link
            "JALR",   // I    Jump & Link Register
            //---------//
            "LUI",    // U    Load Upper Immediate
            "AUIPC",  // U    Add Upper Immediate to PC
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
            "AND",     // R    AND
            "EMPTY",
            "HALT"
    };
    enum OPT {
        NONE,
        //--------MEM----------//
        LB,     // I    Load Byte
        LH,     // I    Load Halfword
        LW,     // I    Load Word
        LBU,    // I    Load Byte Unsigne
        LHU,    // I    Load Halfword Unsigned
        SB,     // S    Store Byte
        SH,     // S    Store Halfword
        SW,     // S    Store Word
        //--------BRANCH----------//
        BEQ,    // SB   Branch Equal
        BNE,    // SB   Branch Not Equal
        BLT,    // SB   Branch Less Than
        BGE,    // SB   Branch Greater than or Equal
        BLTU,   // SB   Branch Less than Unsigned
        BGEU,   // SB   Branch Greater than or Equal Unsigned
        JAL,    // UJ   Jump & Link
        JALR,   // I    Jump & Link Register
        //---------REG------------//
        LUI,    // U    Load Upper Immediate
        AUIPC,  // U    Add Upper Immediate to PC
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
        AND,     // R    AND
        ROBHALT,
        HALT,
    } opt;
    struct IDResult {
        OPT opt;
        u32 imm = 0;
        u8 rd = 0, rs1 = 0, rs2 = 0;
    };

    inline u32 Getrange(int l, int r, u32 so) {//0-base
        u32 ans = (so << (31 - l));
        ans = ans >> (r + (31 - l));
        return ans;
    }

    u32 Signextend(u32 imm, u32 index) {//index??????????????? 0-based
        if (!(imm & (1u << index)))return imm & (0b11111111'11111111'11111111'11111111u >> (31 - index));//if ????????????0
        else {
            return imm | (0b11111111'11111111'11111111'11111111u << (index + 1));
        }
    }

    IDResult Rtype(u32 so, OPT opt) {
        IDResult ans;
        ans.opt = opt;
        ans.rs1 = Getrange(19, 15, so);
        ans.rs2 = Getrange(24, 20, so);
        ans.rd = Getrange(11, 7, so);
        return ans;
    }

    IDResult Utype(u32 so, OPT opt) {
        IDResult ans;
        ans.opt = opt;
        ans.imm = Getrange(31, 12, so) << 12;
        ans.rd = Getrange(11, 7, so);
        return ans;
    }

    IDResult Jtype(u32 so, OPT opt) {
        IDResult ans;
        ans.opt = opt;
        ans.imm = (so & 0b10000000'00000000'00000000'00000000u) >> 11u;   // 20
        ans.imm |= (so & 0b01111111'11100000'00000000'00000000u) >> 20u;   // 10:1
        ans.imm |= (so & 0b00000000'00010000'00000000'00000000u) >> 9u;    // 11
        ans.imm |= (so & 0b00000000'00001111'11110000'00000000u);          // 19:12
        ans.imm = Signextend(ans.imm, 20);
        ans.rd = Getrange(11, 7, so);
        return ans;
    }

    IDResult Itype(u32 so, OPT opt) {
        IDResult ans;
        ans.opt = opt;
        ans.rd = Getrange(11, 7, so);
        ans.imm = Signextend(Getrange(31, 20, so), 11);
        ans.rs1 = Getrange(19, 15, so);
        return ans;
    }

    IDResult Stype(u32 so, OPT opt) {
        IDResult ans;
        ans.opt = opt;
        ans.rs1 = Getrange(19, 15, so);
        ans.rs2 = Getrange(24, 20, so);
//        ans.rd=Getrange(11,7,so);
        ans.imm = Signextend((Getrange(31, 25, so) << 5) + Getrange(11, 7, so), 11);
        return ans;
    }

    IDResult Btype(u32 so, OPT opt) {
        IDResult ans;
        ans.opt = opt;
        ans.rs1 = Getrange(19, 15, so);
        ans.rs2 = Getrange(24, 20, so);
        ans.imm = Signextend((Getrange(31, 31, so) << 12) + (Getrange(30, 25, so) << 5) + (Getrange(11, 8, so) << 1) +
                             (Getrange(7, 7, so) << 11), 12);
        return ans;
    }

    IDResult ID(u32 so) {
        IDResult ans;
        u32 optcode = Getrange(6, 0, so);
        u32 OPT3 = Getrange(14, 12, so);
        u32 OPT4 = Getrange(31, 25, so);

        switch (optcode) {
            case 0b0110111:
                ans = Utype(so, LUI);
                break;
            case 0b0010111:
                ans = Utype(so, AUIPC);
                break;
            case 0b1101111:
                ans = Jtype(so, JAL);
                break;
            case 0b1100111:
                ans = Itype(so, JALR);
                break;
            case 0b1100011://B
                switch (OPT3) {
                    case 0b000:
                        ans = Btype(so, BEQ);
                        break;
                    case 0b001:
                        ans = Btype(so, BNE);
                        break;
                    case 0b100:
                        ans = Btype(so, BLT);

                        break;
                    case 0b101:
                        ans = Btype(so, BGE);
                        break;
                    case 0b110:
                        ans = Btype(so, BLTU);
                        break;
                    case 0b111:
                        ans = Btype(so, BGEU);
                        break;
                    default: {
                        throw ("case 0b1100011 DC");
                    }
                }
                break;
            case 0b0000011://I
                switch (OPT3) {
                    case 0b000:
                        ans = Itype(so, LB);
                        break;
                    case 0b001:
                        ans = Itype(so, LH);
                        break;
                    case 0b010:
                        ans = Itype(so, LW);
                        break;
                    case 0b100:
                        ans = Itype(so, LBU);
                        break;
                    case 101:
                        ans = Itype(so, LHU);
                        break;
                    default:
                        throw ("case 0b0000011 DC!");
                }
                break;
            case 0b0100011://S
                switch (OPT3) {
                    case 0b000:
                        ans = Stype(so, SB);
                        break;
                    case 0b001:
                        ans = Stype(so, SH);
                        break;
                    case 0b010:
                        ans = Stype(so, SW);
                        break;
                    default:
                        throw ("line210 OPT3 NOT reach!");
                }
                break;
            case 0b0010011://I
                switch (OPT3) {
                    case 0b000:
                        ans = Itype(so, ADDI);
                        if (int(ans.rd) == 10 && ans.imm == 255) {
                            ans.opt = HALT;
                        }
                        break;
                    case 0b010:
                        ans = Itype(so, SLTI);
                        break;
                    case 0b011:
                        ans = Itype(so, SLTIU);
                        break;
                    case 0b100:
                        ans = Itype(so, XORI);
                        break;
                    case 0b110:
                        ans = Itype(so, ORI);
                        break;
                    case 0b111:
                        ans = Itype(so, ANDI);
                        break;
                    case 0b001:
                        ans.opt = SLLI;
                        ans.rs1 = Getrange(19, 15, so);
                        ans.rd = Getrange(11, 7, so);
                        ans.imm = Getrange(24, 20, so);
                        break;
                    case 0b101:
                        switch (OPT4) {
                            case 0b0000000:
                                ans.opt = SRLI;
                                ans.rs1 = Getrange(19, 15, so);
                                ans.rd = Getrange(11, 7, so);
                                ans.imm = Getrange(24, 20, so);
                                break;
                            case 0b0100000:
                                ans.opt = SRAI;
                                ans.rs1 = Getrange(19, 15, so);
                                ans.rd = Getrange(11, 7, so);
                                ans.imm = Getrange(24, 20, so);
                                break;
                            default:
                                throw ("line246 OPT3 not reach!");
                        }
                        break;
                }
                break;
            case 0b0110011:
                switch (OPT3) {
                    case 0b000:
                        switch (OPT4) {
                            case 0b0000000:
                                ans = Rtype(so, ADD);
                                break;
                            case 0b0100000:
                                ans = Rtype(so, SUB);
                                break;
                            default:
                                throw ("line313 OPT4 not reach!");
                        }
                        break;
                    case 0b001:
                        ans = Rtype(so, SLL);
                        break;
                    case 0b010:
                        ans = Rtype(so, SLT);
                        break;
                    case 0b011:
                        ans = Rtype(so, SLTU);
                        break;
                    case 0b100:
                        ans = Rtype(so, XOR);
                        break;
                    case 0b101:
                        switch (OPT4) {
                            case 0b0000000:
                                ans = Rtype(so, SRL);
                                break;
                            case 0b0100000:
                                ans = Rtype(so, SRA);
                                break;
                            default:
                                throw ("line274 OPT4 not reach!");
                        }
                        break;
                    case 0b110:
                        ans = Rtype(so, OR);
                        break;
                    case 0b111:
                        ans = Rtype(so, AND);
                        break;
                    default:
                        throw ("line 280 OPT3 not reach!");
                }
                break;
            default:
                throw ("line 282 OPT not reach!");
        }
        return ans;
    }

    IDResult getopt() {
        //write into RS&ROB&LSB
        if(jump_wrong){
            PC=change_pc_to;
            jump_wrong=false;
        }
        u32 point = PC;
//        std::cout << std::hex << PC<<' '<<pre_register[10]<< std::endl;
        u32 so = u32(Memory[point]) + (u32(Memory[point + 1]) << 8) + (u32(Memory[point + 2]) << 16) +
                 (u32(Memory[point + 3]) << 24);
        IDResult ans = ID(so);
        return ans;
    }

    bool rsfull() {
        int num = 0;
        for (int i = 0; i < RS_SIZE; ++i)if (nowRS[i].busy)++num;
        return num == RS_SIZE;
    }

    template<class K>
    class queue {//???????????????????????????????????????????????????????????????
    public://????????????????????????
        K *a;
        int front = 0, rear = 0;
        int maxSize;

        explicit queue(int size = QUEUE_SIZE) {
            maxSize = size;
            a = new K[size];
        }

        ~queue() {
            delete[] a;
        };

        queue &operator=(const queue &q) {
            for (int i = 0; i < q.maxSize; ++i) {
                a[i] = q.a[i];
            }
            front = q.front;
            rear = q.rear;
            return *this;
        }

        K pop() {
            front = (front + 1) % maxSize;
            return a[front];
        }

        K top() {
            return a[(front + 1) % maxSize];
        }

        int in(K value) {
            rear = (rear + 1) % maxSize;
            a[rear] = value;
            return rear;
        }

        bool isfull() {
            return ((rear + 1) % maxSize == front);
        }

        bool isempty() {
            return (front == rear);
        }

        void clear() {
            rear = front;
        }
    };

    struct ALU {
        u32 value = 0;
        int reorder = -1;
        u32 pc = 0;
        OPT opt = NONE;
    } preALU, nowALU;

    struct ROBnode {
        bool ready = false;
        bool isjumped=false;
        OPT opt;
        u8 rd;
        u32 value = 0;
        u32 beqpc;
        u32 pc_;
    };

    queue<ROBnode> preROB;

    queue<ROBnode> nowROB;

    bool check_ready(u32 index) {//???rob?????????????????????ready
        if (nowROB.a[index].ready)return true;
        return false;
    }

    struct RSnode {
        OPT op;
        bool busy = false;//true???????????????????????????????????????ALU
        bool isready = false;
        u32 Vj = 0, Vk = 0;
        int Qj = -1, Qk = -1;
        u32 reorder;
        int beq;//???????????????pc???
        int pc_;
    } preRS[RS_SIZE], nowRS[RS_SIZE];

    struct LSnode {
        OPT opt;
        u32 Vj, Vk;
        u32 reorder;
        int pc_;
        int Qj = -1;
        int Qk = -1;
        bool isload = false;
        bool ready = false;
        bool isdelete = false;
    };

    queue<LSnode> preLS;

    queue<LSnode> nowLS;

    void rs_get_issue(IDResult op, u32 reorder) {
        if (op.opt == LUI) {
            PC += 4;//LUI???????????????????????????,???????????????pc???
            return;
        }
        if (op.opt == HALT)return;
        RSnode node;
        node.reorder = reorder;
        node.op = op.opt;
        node.busy = true;
        node.pc_ = PC;
        if (op.opt >= BEQ && op.opt <= JALR) {//for branch
            if (op.opt == JAL) {
                PC += op.imm;
                return;
            } else if (op.opt == JALR) {
                issue_halt = true;
                node.Vk = op.imm;
                if (pre_rename[u32(op.rs1)] == -1) {
                    node.Vj = pre_register[u32(op.rs1)];
                } else if (check_ready(pre_rename[u32(op.rs1)])) {
                    node.Vj = nowROB.a[pre_rename[u32(op.rs1)]].value;
                } else node.Qj = pre_rename[u32(op.rs1)];
                if (node.Qj == -1 && node.Qk == -1)node.isready = true;
            } else {
                if(branch_predictor.jump_predict(PC)){
                    node.beq=PC+4;
                    PC+=op.imm;
                    nowROB.a[reorder].isjumped=true;
                }
                else{
                    node.beq=PC+op.imm;
                    PC+=4;
                    nowROB.a[reorder].isjumped=false;
                }
//                node.beq = PC+4; //???????????????
                if (pre_rename[u32(op.rs1)] == -1) {
                    node.Vj = pre_register[u32(op.rs1)];
                } else if (check_ready(pre_rename[u32(op.rs1)])) {
                    node.Vj = nowROB.a[pre_rename[u32(op.rs1)]].value;
                } else node.Qj = pre_rename[u32(op.rs1)];

                if (pre_rename[u32(op.rs2)] == -1) {
                    node.Vk = pre_register[u32(op.rs2)];
                } else if (check_ready(pre_rename[u32(op.rs2)])) {
                    node.Vk = nowROB.a[pre_rename[u32(op.rs2)]].value;
                } else node.Qk = pre_rename[u32(op.rs2)];
                if (node.Qj == -1 && node.Qk == -1)node.isready = true;
            }
        } else if (op.opt >= LB && op.opt <= SW) {//for mem
            if (op.opt >= LB && op.opt <= LHU)node.reorder = reorder + 256;
            node.Vk = op.imm;
            if (pre_rename[u32(op.rs1)] == -1) {
                node.Vj = pre_register[u32(op.rs1)];
                node.isready = true;
            } else if (check_ready(pre_rename[u32(op.rs1)])) {
                node.Vj = nowROB.a[pre_rename[u32(op.rs1)]].value;
                node.isready = true;
            } else node.Qj = pre_rename[u32(op.rs1)];
            PC += 4;
        } else if (op.opt > LUI) {
            if (op.opt == AUIPC) {
                node.Vj = PC;
                node.Vk = op.imm;
            } else if (op.opt == HALT)return;
            else {
                if (pre_rename[u32(op.rs1)] == -1) {
                    node.Vj = pre_register[u32(op.rs1)];
                } else if (check_ready(pre_rename[u32(op.rs1)])) {
                    node.Vj = nowROB.a[pre_rename[u32(op.rs1)]].value;
                } else node.Qj = pre_rename[u32(op.rs1)];
                if (op.opt >= ADDI && op.opt <= SRAI)node.Vk = op.imm;
                else if (op.opt > SRAI && op.opt <= AND) {
                    if (pre_rename[u32(op.rs2)] == -1) {
                        node.Vk = pre_register[u32(op.rs2)];
                    } else if (check_ready(pre_rename[u32(op.rs2)])) {
                        node.Vk = nowROB.a[pre_rename[u32(op.rs2)]].value;
                    } else {
                        node.Qk = pre_rename[u32(op.rs2)];
                    }
                }
                if (node.Qj == -1 && node.Qk == -1)node.isready = true;
            }
            PC += 4;
        }
        int i;
        for (i = 0; i < RS_SIZE; ++i) {
            if (!nowRS[i].busy) {
                nowRS[i] = node;
                nowRS[i].busy = true;
                break;
            }
        }
//        cout << "issue "<<PP[node.op] << ' '<< node.Qj <<' '<<node.Qk<<' '<<node.Vk<<' '<<reorder << endl;
        if (i == RS_SIZE)throw ("rs is full");
    }
    void rs_send_ready() {
        int i = 0;
        for (i = 0; i < RS_SIZE; ++i) {
            if (preRS[i].busy) {
                if (preRS[i].Qj == -1 && preRS[i].Qk == -1) { // TODO: isready
                    nowALU.reorder = preRS[i].reorder;
                    u32 rs1 = preRS[i].Vj;
                    u32 rs2 = preRS[i].Vk;
                    u32 imm = preRS[i].Vk;
                    u32 pc = preRS[i].beq;
                    OPT opt = preRS[i].op;
                    nowALU.opt = preRS[i].op;
                    if (opt >= LB && opt <= SW) {
                        nowALU.value = rs1 + imm;
                    } else {
                        switch (opt) {
                            case ADD:
                                nowALU.value = rs1 + rs2;
                                break;
                            case ADDI:
                                nowALU.value = rs1 + rs2;
                                break;
                            case SUB:
//                                if(preRS[i].pc_==0x1120)cout << rs1 << ' ' << rs2 << "@@@"<<endl;
                                nowALU.value = rs1 - rs2;
                                break;
                            case LUI://???????????????
                                break;
                            case AUIPC:
                                nowALU.value = rs1 + imm;
                                break;
                            case XOR:
                                nowALU.value = rs1 ^ rs2;
                                break;
                            case XORI:
                                nowALU.value = rs1 ^ imm;
                                break;
                            case OR:
//                                cout << rs1 << ' ' << rs2 << endl;
                                nowALU.value = rs1 | rs2;
                                break;
                            case ORI:
                                nowALU.value = rs1 | imm;
                                break;
                            case AND:
                                nowALU.value = rs1 & rs2;
                                break;
                            case ANDI:
                                nowALU.value = rs1 & imm;
                                break;
                            case SLL:
                                nowALU.value = rs1 << rs2;
                                break;
                            case SLLI:
                                nowALU.value = rs1 << imm;
                                break;
                            case SRL:
                                nowALU.value = rs1 >> rs2;
                                break;
                            case SRLI:
                                nowALU.value = rs1 >> imm;
                                break;
                            case SRA:
                                nowALU.value = Signextend(rs1 >> rs2, 31 - rs2);
                                break;
                            case SRAI:
                                nowALU.value = Signextend(rs1 >> rs2, 31 - rs2);
                                break;
                            case SLT:
                                nowALU.value = (int(rs1) < int(rs2));
                                break;
                            case SLTI:
                                nowALU.value = (int(rs1) < int(rs2));
                                break;
                            case SLTU:
                                nowALU.value = (rs1 < rs2);
                                break;
                            case SLTIU:
                                nowALU.value = (rs1 < rs2);
                                break;
                            case BEQ:
                                nowALU.value = (rs1 == rs2);
                                nowALU.pc = pc;
                                break;
                            case BNE:
                                nowALU.value = (rs1 != rs2);
                                nowALU.pc = pc;
                                break;
                            case BLT:
//                                cout << "rs1 rs2 " <<dec<< int(rs1) << ' ' << int(rs2) <<' ';
                                nowALU.value = (int(rs1) < int(rs2));
//                                cout << nowALU.value << endl;
                                nowALU.pc = pc;
                                break;
                            case BGE:
                                nowALU.value = (int(rs1) >= int(rs2));
                                nowALU.pc = pc;
                                break;
                            case BLTU:
//                                cout << "%%%%" <<preRS[i].pc_<<' '<< rs1 << ' ' << rs2 << endl;
                                nowALU.value = (rs1 < rs2);
                                nowALU.pc = pc;
                                break;
                            case BGEU:
                                nowALU.value = (rs1 >= rs2);
                                nowALU.pc = pc;
                                break;
                            case JALR:
                                nowALU.pc = (rs1 + imm) & (~1);
                                if (!nowALU.pc)throw ("pc is zeor!");
                                break;
                            case LB:
                                nowALU.value = rs1 + imm;
                                break;
                            case LH:
                                nowALU.value = rs1 + imm;
                                break;
                            case LW:
                                nowALU.value = rs1 + imm;
                                break;
                            case LBU:
                                nowALU.value = rs1 + imm;
                                break;
                            case LHU:
                                nowALU.value = rs1 + imm;
                                break;
                            case SB:
                                nowALU.value = rs1 + imm;
                                break;
                            case SH:
                                nowALU.value = rs1 + imm;
                                break;
                            case SW:
                                nowALU.value = rs1 + imm;
                                break;
                            case NONE:
                                break;
                            case JAL:
                                break;
                            case ROBHALT:
                                break;
                            case HALT:
                                break;
                        }
                    }
                    nowRS[i].busy = nowRS[i].isready = false;
                    nowRS[i].Qj = nowRS[i].Qk = -1;
                    break;
                }
            }
        }
        if (i == RS_SIZE)nowALU.opt = NONE;
    }
    void rs_listen() {
        //branch
        if (preALU.reorder == -1||preALU.opt==NONE)return;
//        cout <<"rslisten "<<preALU.reorder<<' '<<preALU.value<<endl;
        if (preALU.opt == JALR) {
            issue_halt = false;
            PC = preALU.pc;
            return;
        }
        //------beqoff-------//
//        if (preALU.opt >= BEQ && preALU.opt <= BGEU) {
//            issue_halt = false;
//            if (preALU.value) {
//                PC = preALU.pc;
//            } else PC += 4;
//            return;
//        }
        //--------------------//
        if (preALU.opt >= LB && preALU.opt <= LHU)return;
        for (int i = 0; i < RS_SIZE; ++i) {
            if (preRS[i].busy) {
                if (preRS[i].Qj == preALU.reorder) {
                    nowRS[i].Vj = preALU.value;
                    nowRS[i].Qj = -1;
//                    if(preALU.reorder==3)cout<<nowRS[i].reorder<<' ' <<nowRS[i].Vj<<' '<<nowRS[i].Vk<<' '<<i<<"qwq"<<endl;
                }
                if (preRS[i].Qk == preALU.reorder && preRS[i].op > LHU) {//TODO &&??????????????????
                    nowRS[i].Vk = preALU.value;
                    nowRS[i].Qk = -1;
//                    if(preALU.reorder==3)cout <<nowRS[i].Vj<<' '<<nowRS[i].Vk<<' '<<i<<' ' <<"qwq"<<endl;
                }
                if (nowRS[i].Qj == -1 && nowRS[i].Qk == -1)nowRS[i].isready = true;
//                if(preALU.reorder==3)cout<<i<<' ' <<nowRS[i].Vj<<' '<<nowRS[i].Vk<<' '<<nowRS[i].Qj<<' '<<nowRS[i].Qk<<' ' <<"qwq"<<endl;
            }
        }
    }

    void lsb_get_issue(IDResult op, u32 reorder) {
        LSnode node;
        node.pc_ = PC;
        node.reorder = reorder;
        node.opt = op.opt;
        if (op.opt < LB || op.opt > SW)return;
        if (op.opt >= LB && op.opt <= LHU) {
            node.isload = true;
            node.Qj = reorder + 256;
            node.ready = true;
        } else {
            node.Qj = reorder;
//            if (op.opt == SW) cout << " &&&" << reorder << endl;
            if (pre_rename[u32(op.rs2)] == -1) {
                node.Vk = pre_register[u32(op.rs2)];
            } else if (check_ready(pre_rename[u32(op.rs2)])) {
                node.Vk = nowROB.a[pre_rename[u32(op.rs2)]].value;
            } else node.Qk = pre_rename[u32(op.rs2)];
        }
        nowLS.in(node);
    }
    void lsb_listen() {
        if (preALU.opt == NONE) return;
        if (preALU.reorder == -1)return;
//        cout << "lsblisten"<<preALU.reorder <<' '<<preALU.value<<endl;
        for (int i = (preLS.front + 1) % preLS.maxSize;
             i != (preLS.rear + 1) % preLS.maxSize; i = (i + 1) % preLS.maxSize) {
            if (preLS.a[i].Qj == preALU.reorder) {
                nowLS.a[i].Vj = preALU.value;
                nowLS.a[i].Qj = -1;
            }
            if (preLS.a[i].Qk == preALU.reorder) {
                nowLS.a[i].Vk = preALU.value;
                nowLS.a[i].Qk = -1;
            }
        }
    }
    void ls_clear() {
        for (int i = (nowLS.front + 1) % nowLS.maxSize;
             i != (nowLS.rear + 1) % nowLS.maxSize; i = (i + 1) % nowLS.maxSize) {
            if (!nowLS.a[i].isload && !nowLS.a[i].ready)nowLS.a[i].isdelete = true;
            else if (nowLS.a[i].isload)nowLS.a[i].isdelete = true;
        }
        for (int i = (preLS.front + 1) % preLS.maxSize;
             i != (preLS.rear + 1) % preLS.maxSize; i = (i + 1) % preLS.maxSize) {
            if (!preLS.a[i].isload && !preLS.a[i].ready)preLS.a[i].isdelete = true;
            else if (preLS.a[i].isload)preLS.a[i].isdelete = true;
        }
    }
    void lsb_commit() {
        LSnode node;
        if (preLS.isempty())return;
        node = preLS.top();
        while (node.isdelete) {
            preLS.pop();
            nowLS.pop();
            if(!preLS.isempty())node=preLS.top();
            else break;
        }
        if (node.isdelete)return;
        if (node.isload) {
            if (node.Qj == -1 && node.Qk == -1 && node.ready) {
                if (lsb_clk >= 2) {
                    nowLS.pop();
                    nowROB.a[node.reorder].ready = true;
                    switch (node.opt) {
                        case LW:
                            nowROB.a[node.reorder].value = u32(Memory[node.Vj]) + (u32(Memory[node.Vj + 1]) << 8) +
                                                           (u32(Memory[node.Vj + 2]) << 16) +
                                                           (u32(Memory[node.Vj + 3]) << 24);
                            break;
                        case LB:
                            nowROB.a[node.reorder].value = Signextend(u32(Memory[node.Vj]), 7);
                            break;
                        case LH:
                            nowROB.a[node.reorder].value = Signextend(
                                    u32(Memory[node.Vj]) + (u32(Memory[node.Vj + 1]) << 8), 15);
                            break;
                        case LBU:
                            nowROB.a[node.reorder].value = u32(Memory[node.Vj]);
                            break;
                        case LHU:
                            nowROB.a[node.reorder].value = u32(Memory[node.Vj]) + (u32(Memory[node.Vj + 1]) << 8);
                            break;
                    }
                    lsb_clk = 0;
                    for (int i = 0; i < RS_SIZE; ++i) {
                        if (preRS[i].busy) {
                            if (preRS[i].Qk == node.reorder) {
                                nowRS[i].Qk = -1;
                                nowRS[i].Vk = nowROB.a[node.reorder].value;
                            }
                            if (preRS[i].Qj == node.reorder) {
                                nowRS[i].Qj = -1;
                                nowRS[i].Vj = nowROB.a[node.reorder].value;
                            }
                            if (nowRS[i].Qk == -1 && nowRS[i].Qj == -1)nowRS[i].isready = true;
                        }
                    }
                    for (int i = (preLS.front + 1) % preLS.maxSize;
                         i != (preLS.rear + 1) % preLS.maxSize; i = (i + 1) % preLS.maxSize) {
                        if (preLS.a[i].Qk == node.reorder) {
                            nowLS.a[i].Qk = -1;
                            nowLS.a[i].Vk = nowROB.a[node.reorder].value;
                        }
                        if (preLS.a[i].Qj == node.reorder) {
                            nowLS.a[i].Qj = -1;
                            nowLS.a[i].Vj = nowROB.a[node.reorder].value;
                        }
                    }
                    // cout << "lsb commit L" << PP[node.opt] << ' ' << node.reorder << endl;
                } else {
                    ++lsb_clk;
                }
            }
        } else {
//            cout << node.reorder << "#####" << " " << node.Qj << " " << node.Qk << " " << node.ready << " " << lsb_clk<<endl;
            if (node.Qj == -1 && node.Qk == -1 && node.ready) {
                if(lsb_clk<=-2){
                    nowLS.pop();
                    switch(node.opt){
                        case SB:
                            Memory[node.Vj] = u8(node.Vk);
                            break;
                        case SH:
                            Memory[node.Vj] = u8(node.Vk);
                            Memory[node.Vj + 1] = u8(node.Vk >> 8);
                            break;
                        case SW:
                            Memory[node.Vj] = node.Vk & 0xff;
                            Memory[node.Vj + 1] = (node.Vk >> 8) & 0xff;
                            Memory[node.Vj + 2] = (node.Vk >> 16) & 0xff;
                            Memory[node.Vj + 3] = (node.Vk >> 24) & 0xff;
                            break;
                    }
                    lsb_clk=0;
                }
                else --lsb_clk;
            }
        }
    }

    int rob_get_issue(IDResult op) {
        ROBnode node;
        node.opt = op.opt;
        node.pc_ = PC;
//----------beqoff----------//
        if (op.opt >= BEQ && op.opt <= BGEU) {
            node.ready = false;
        }
//----------------//

        if (op.opt == LUI) {
            node.ready = true;
            node.value = op.imm;
        }
        if (op.opt == JALR || op.opt == JAL) {
            node.ready = true;
            node.value = PC + 4;
        }
        if (op.opt == HALT)node.ready = true;
        node.rd = op.rd;
        int reorder = nowROB.in(node);
        if (op.opt >= LB && op.opt <= LHU || op.opt >= JAL && op.opt < ROBHALT) {
            now_rename[op.rd] = reorder;
        }

        return reorder;
    }
    void rob_listen() {
        // cout << "rob listen " << ' ' << preALU.reorder<<' '<<PP[preALU.opt] << ' ';
        if (preALU.opt == NONE) return;
        if (preALU.reorder != -1) {
            if (preROB.a[preALU.reorder].opt == JALR || preROB.a[preALU.reorder].opt == JAL) {
                return;
            } else if (preROB.a[preALU.reorder].opt > LHU && preROB.a[preALU.reorder].opt <= AND) {//??????load
                nowROB.a[preALU.reorder].beqpc = preALU.pc;
                nowROB.a[preALU.reorder].value = preALU.value;
                nowROB.a[preALU.reorder].ready = true;
            }
        }
    }
    OPT rob_commit() {
        if (preROB.isempty())return NONE;
        ROBnode node = preROB.top();
        if (node.opt == HALT)return HALT;
        int bug=11;
        if (node.ready) {
            ++clk;
//            if(clk==200)exit(0);
            // TODO BRANCH CLEAR FIXIT
            //------------------beqoff----------------//
            if (node.opt>=BEQ&&node.opt<=BGEU && node.value==node.isjumped) {//????????????
                nowROB.pop();
//                cout << branch_predictor.predict[node.pc_&branch_predictor.M]<<"()()"<<endl;
                branch_predictor.result(node.pc_,node.value);
//                cout <<hex<< node.pc_<<" right "<<node.beqpc<<' '<<node.isjumped<< endl;
//cout << hex << node.pc_ << endl;
                jump_wrong=false;
                return node.opt;
            } else if (node.opt>=BEQ&&node.opt<=BGEU && node.value!=node.isjumped) {//???????????????
//                cout <<hex<< node.pc_<<" wrong "<<node.beqpc<<' '<<node.isjumped<< endl;
                nowROB.pop();
                branch_predictor.result(node.pc_,node.value);
                beq_faild();
                change_pc_to=node.beqpc;
                jump_wrong=true;
//                cout <<hex<< node.pc_<< endl;
                return node.opt;
            }
            //---------------------//
            if (node.opt >= LB && node.opt <= LHU) {//isload,update rd ????????????
                now_register[node.rd] = node.value;
                if (pre_rename[node.rd] == (preROB.front + 1) % preROB.maxSize)now_rename[node.rd] = -1;
                nowROB.pop();
//                cout <<hex<< node.pc_<<endl;
//                cout << "CM "<<(preROB.front+1)%preROB.maxSize<<' '<<PP[node.opt]<<' '<<u32(node.rd) << ' '<<hex<<now_register[node.rd] <<endl;
            } else if (node.opt >= SB && node.opt <= SW) {//isstore
                //send to lsbuffer
                int reorder = (preROB.front + 1) % preROB.maxSize;
                for (int i = (preLS.front + 1) % preLS.maxSize;
                     i != (preLS.rear + 1) % preLS.maxSize; i = (i + 1) % preLS.maxSize) {
                    if (preLS.a[i].reorder == reorder&&preLS.a[i].Qj==-1&&preLS.a[i].Qk==-1&&!preLS.a[i].ready&&!preLS.a[i].isdelete) {
                        nowLS.a[i].ready = true;
                        nowROB.pop();
                        break;
                    }
                }
//                cout <<hex<< node.pc_<<endl;
//                cout << "CM "<<(preROB.front+1)%preROB.maxSize<<' '<<PP[node.opt]<<' '<<u32(node.rd) << ' '<<node.value <<endl;
            } else {
                nowROB.pop();
                now_register[node.rd] = node.value;
                if (pre_rename[node.rd] == (preROB.front + 1) % preROB.maxSize)now_rename[node.rd] = -1;
//                cout <<hex<< node.pc_<<endl;
//                cout << "CM "<<(preROB.front+1)%preROB.maxSize<<' '<<PP[node.opt]<<' '<<u32(node.rd) << ' '<<now_register[node.rd] <<endl;
            }
            return node.opt;
        }
        return ROBHALT;
    }

public:
    simulator() {}

    void parser(std::istream &Input) {
        u32 point = 0;
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

    void init(std::istream &Input) {
        memset(Memory, 0, sizeof(Memory));
        parser(Input);
        memset(pre_register, 0, sizeof(pre_register));
        memset(now_register, 0, sizeof(pre_register));
        for (int i = 0; i < 32; ++i)pre_rename[i] = now_rename[i] = -1;
        PC = 0;
    }

    void run() {
        OPT opt = ADDI;
        while (opt != HALT){
            update();
            opt = run_rob();
            run_slbuffer();
            run_reservation();//pc==0
            run_issue();
        }
        std::cout << dec << (now_register[10] & 255u) << std::endl;
    }

    void run_issue() {//???????????????????????????
//        cout << issue_halt << "####" << endl;
        if (issue_halt)return;
        if (nowROB.isfull() || rsfull() || nowLS.isfull()) {
            LSnode node=preLS.top();
//            cout <<lsb_clk<< "LT "<<hex<<node.pc_<<' '<<PP[node.opt]<<' '<< node.Vj <<' ' << node.Vk<<' '<<node.Qj<<' '<<node.Qk<<' '<<node.ready << endl;
//            cout << nowROB.isfull() << rsfull() << nowLS.isfull()<<endl;
            return;
        }
        IDResult option = getopt();
//        cout << PP[option.opt] << endl;
        int reorder = rob_get_issue(option);
        rs_get_issue(option, reorder);
        lsb_get_issue(option, reorder);
    }

    void beq_faild() {
        for (int i = 0; i < 32; ++i)now_rename[i] =pre_rename[i]=-1;
        for(int i = 0; i < RS_SIZE; ++i)nowRS[i].busy=preRS[i].busy=false;
        nowROB.clear();
        preROB.clear();
        ls_clear();
//        preLS = nowLS;
        issue_halt=false;
        nowALU.opt=preALU.opt=NONE;
        nowALU.reorder=preALU.reorder=-1;
        if(lsb_clk>0)lsb_clk=0;
    }

    void run_reservation() {
        rs_send_ready();
        rs_listen();
    }

    void run_slbuffer() {
        //-------listen--------//
        //ROB?????????LSB????????????
        lsb_listen();
        //--------commit--------//
        lsb_commit();

    }

    OPT run_rob() {
        //--------listen--------//
        rob_listen();
        //------commit------//
        OPT tmp = rob_commit();
        return tmp;
    }

    void update() {
        now_rename[0] = -1;
        now_register[0] = 0;
        for (int i = 0; i < REG_SIZE; ++i) {
            pre_rename[i] = now_rename[i];
            pre_register[i] = now_register[i];
        }
        for (int i = 0; i < RS_SIZE; ++i) {
            preRS[i] = nowRS[i];
        }
        preROB = nowROB;
        preLS = nowLS;
        preALU = nowALU;
    }

    ~simulator() {}
};
#endif //PPCA_TEST_TOMASULO_HPP