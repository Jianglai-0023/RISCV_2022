#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <iostream>

//const int QUEUE_SIZE = 32;
//const int mem_size = 4194304;
using u32 = unsigned int;
using u8 = unsigned char;
const int QUEUE_SIZE=32;
class simulator{
private:
    u8 Memory[1048576];
    u32 pre_register[32];
    u32 now_register[32];
    u32 pre_rename[32];
    u32 now_rename[32];
    u32 PC;
    bool issue_halt=false;
    enum OPT {
        HALT,
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
        AND     // R    AND
    } opt;
    struct IDResult{
        OPT opt;
        u32 imm=0;
        u8 rd,rs1,rs2;
    };
    inline u32 Getrange(int l, int r, u32 so) {//0-base
        u32 ans = (so<<(31-l));
        ans=ans>>(r+(31-l));
        return ans;
    }
    u32 Signextend(u32 imm,u32 index){//index为非零下标 0-based
        if(!(imm&(1u<<index)))return imm&(0b11111111'11111111'11111111'11111111u >> (31-index));//if 符号位是0
        else{
            return imm|(0b11111111'11111111'11111111'11111111u << (index + 1));
        }
    }
    IDResult Rtype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rs1=Getrange(19,15,so);
        ans.rs2=Getrange(24,20,so);
        ans.rd=Getrange(11,7,so);
        return ans;
    }
    IDResult Utype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.imm= Getrange(31,12,so)<<12;
        ans.rd= Getrange(11,7,so);
        return ans;
    }
    IDResult Jtype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.imm  = (so & 0b10000000'00000000'00000000'00000000u) >> 11u;   // 20
        ans.imm |= (so & 0b01111111'11100000'00000000'00000000u) >> 20u;   // 10:1
        ans.imm |= (so & 0b00000000'00010000'00000000'00000000u) >> 9u;    // 11
        ans.imm |= (so & 0b00000000'00001111'11110000'00000000u);          // 19:12
        ans.imm=Signextend(ans.imm,20);
        ans.rd=Getrange(11,7,so);
        return ans;
    }
    IDResult Itype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rd= Getrange(11,7,so);
        ans.imm= Signextend(Getrange(31,20,so),11);
        ans.rs1= Getrange(19,15,so);
        return ans;
    }
    IDResult Stype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rs1=Getrange(19,15,so);
        ans.rs2=Getrange(24,20,so);
//        ans.rd=Getrange(11,7,so);
        ans.imm=Signextend((Getrange(31,25,so)<<5)+Getrange(11,7,so),11);
        return ans;
    }
    IDResult Btype(u32 so,OPT opt){
        IDResult ans;
        ans.opt=opt;
        ans.rs1= Getrange(19,15,so);
        ans.rs2=Getrange(24,20,so);
        ans.imm= Signextend((Getrange(31,31,so)<<12)+(Getrange(30,25,so)<<5)+(Getrange(11,8,so)<<1)+(Getrange(7,7,so)<<11),12);
        return ans;
    }
    IDResult ID(u32 so) {
        IDResult ans;
        u32 optcode = Getrange(6, 0, so);
        u32 OPT3 = Getrange(14, 12, so);
        u32 OPT4 = Getrange(31, 25, so);
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
                        ans.rs1=Getrange(19,15,so);
                        ans.rd=Getrange(11,7,so);
                        ans.imm=Getrange(24,20,so);break;
                    case 0b101:
                        switch (OPT4) {
                            case 0b0000000:
                                ans.opt = SRLI;
                                ans.rs1=Getrange(19,15,so);
                                ans.rd=Getrange(11,7,so);
                                ans.imm=Getrange(24,20,so);break;
                            case 0b0100000:
                                ans.opt = SRAI;
                                ans.rs1=Getrange(19,15,so);
                                ans.rd=Getrange(11,7,so);
                                ans.imm=Getrange(24,20,so);break;
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
    IDResult getopt(){
        //write into RS&ROB&LSB
        u32 point = PC;
        u32 so=u32(Memory[point])+(u32(Memory[point + 1])<<8)+(u32(Memory[point + 2])<<16)+(u32(Memory[point + 3])<<24);
        IDResult ans = ID(so);
        return ans;
    }
    template<class K>
    class queue {//循环队列，不可扩容，支持下标访问，编号重复
    public://考虑队列满的情况

        K *a;
        int num = 0;
        int front = 0, rear = 0;
        int maxSize;
    public:
        explicit queue(int size = QUEUE_SIZE) {
            maxSize = size;
            a = new K[size];
        }

        ~queue() {
            delete[] a;
        };

        K pop() {
            front = (front + 1) % maxSize;
            return a[front];
        }

        K top() {
            return a[(front+1)%maxSize];
        }

        int in(K value) {
            rear = (rear + 1) % maxSize;
            a[rear] = node(value);
            return rear;
        }

        bool isfull() {
            return ((rear + 1) % maxSize == front);
        }

    };

        struct ROBnode{
            bool isload=false;
            bool ready=false;
            u8 rd;
            u32 value;
            u32 index;
        };
        queue<ROBnode> preROB;
        queue<ROBnode> nowROB;
        int rob_get_issue(IDResult op,u32 index){
             ROBnode node;
             if(op.opt>=LB&&op.opt<=LHU)node.isload=true;
             node.rd=op.rd;
             node.index=index;
             return nowROB.in(node);
        }
        bool check_ready(u32 index){//去rob队列里观察是否ready
            for(int i = preROB.front;i!=preROB.rear;i=(i+1)%preROB.maxSize)if(i==index&&preROB.a[i].ready)return true;
            return false;
        }

       struct RSnode{
            OPT op;
           bool busy=false;
           bool isMEM=false;
           bool isbeq=false;
           bool isready=false;
           u32 Vj=0,Vk=0;
           int Qj=-1,Qk=-1;
           u32 reorder;
           u32 imm=0;
       };
       queue<RSnode> preRS;
       queue<RSnode> nowRS;
       void rs_get_issue(IDResult op,u32 reorder){
          RSnode node;
          node.reorder=reorder;
          if(op.opt>=BEQ&&op.opt<=JALR){//for branch
            node.isbeq=true;
            if(op.opt==JAL||op.opt==JALR)issue_halt=true;
              if(!pre_rename[u32(op.rs1)]||check_ready(u32(op.rs1))){
                  node.Vj=pre_register[u32(op.rs1)];
              }
              else node.Qj=pre_rename[u32(op.rs1)];
              if(!pre_rename[u32(op.rs2)]||check_ready(u32(op.rs2))){
                  node.Vk=pre_register[u32(op.rs2)];
              }
              else node.Qk=pre_rename[u32(op.rs2)];
              if(node.Qj!=-1&&node.Qk!=-1)node.isready=true;
          }
          else if(op.opt>=LB&&op.opt<=LHU){//mem 只考虑load rs1 imm
                node.imm=op.imm;
              node.isMEM=true;
                if(!pre_rename[u32(op.rs1)]||check_ready(u32(op.rs1))){
                    node.Vj=pre_register[u32(op.rs1)];
                    node.isready=true;
                }
                else node.Qj=pre_rename[u32(op.rs1)];
          }
          else{

          }
          if(!op.imm){//无立即数
              if(!pre_rename[u32(op.rs1)])node.Vj=pre_register[u32(op.rs1)];
              else{
                  if(check_ready(op.rs1))node.Vj=preROB.a[op.rs1].value;
              }
              if(!pre_rename[u32(op.rs2)])node.Vj=pre_register[u32(op.rs2)];
              else{
                  if(check_ready(op.rs2))node.Vj=preROB.a[op.rs2].value;
              }
          }
          else{//有立即数

          }
       }

public:
    simulator(){}
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
    void init(std::istream &Input) {
        memset(Memory,0,sizeof(Memory));
        parser(Input);
        memset(pre_register,0,sizeof(pre_register));
        memset(now_register,0,sizeof(pre_register));
        memset(pre_rename,0,sizeof(pre_rename));
        memset(now_rename,0,sizeof(now_rename));
        PC = 0;
    }
    void run(){
        while(true){
            /*在这里使用了两阶段的循环部分：
              1. 实现时序电路部分，即在每个周期初同步更新的信息。
              2. 实现逻辑电路部分，即在每个周期中如ex、issue的部分
              已在下面给出代码
            */
            update();

            run_rob();
            run_slbuffer();
            run_reservation();

            run_ex();
            run_issue();
            run_commit();
        }

    }
    

    void run_issue(){//获取指令与指令分发
        /*
        在这一部分你需要完成的工作：
        1. 从run_inst_fetch_queue()中得到issue的指令
        2. 对于issue的所有类型的指令向ROB申请一个位置（或者也可以通过ROB预留位置），并修改regfile中相应的值
        2. 对于 非 Load/Store的指令，将指令进行分解后发到Reservation Station
          tip: 1. 这里需要考虑怎么得到rs1、rs2的值，并考虑如当前rs1、rs2未被计算出的情况，参考书上内容进行处理
               2. 在本次作业中，我们认为相应寄存器的值已在ROB中存储但尚未commit的情况是可以直接获得的，即你需要实现这个功能
                  而对于rs1、rs2不ready的情况，只需要stall即可，有兴趣的同学可以考虑下怎么样直接从EX完的结果更快的得到计算结果
        3. 对于 Load/Store指令，将指令分解后发到SLBuffer(需注意SLBUFFER也该是个先进先出的队列实现)
        tips: 考虑边界问题（是否还有足够的空间存放下一条指令）
        */
        if(issue_halt)return;
        IDResult option=getopt();
        OPT opt0=option.opt;
        //LS指令放入LSB
        //load以及非LS指令放入ROB
        //所有指令放入RS
        
    }

    void run_reservation(){
        /*
        在这一部分你需要完成的工作：
        1. 设计一个Reservation Station，其中要存储的东西可以参考CAAQA或其余资料，至少需要有用到的寄存器信息等
        2. 如存在，从issue阶段收到要存储的信息，存进Reservation Station（如可以计算也可以直接进入计算）
        3. 从Reservation Station或者issue进来的指令中选择一条可计算的发送给EX进行计算
        4. 根据上个周期EX阶段或者SLBUFFER的计算得到的结果遍历Reservation Station，更新相应的值
        */
    }

    void run_ex(){
        /*
        在这一部分你需要完成的工作：
        根据Reservation Station发出的信息进行相应的计算
        tips: 考虑如何处理跳转指令并存储相关信息
              Store/Load的指令并不在这一部分进行处理
        */
    }

    void run_slbuffer(){
        /*
        在这一部分中，由于SLBUFFER的设计较为多样，在这里给出两种助教的设计方案：
        1. 1）通过循环队列，设计一个先进先出的SLBUFFER，同时存储 head1、head2、tail三个变量。
           其中，head1是真正的队首，记录第一条未执行的内存操作的指令；tail是队尾，记录当前最后一条未执行的内存操作的指令。
           而head2负责确定处在head1位置的指令是否可以进行内存操作，其具体实现为在ROB中增加一个head_ensure的变量，每个周期head_ensure做取模意义下的加法，直到等于tail或遇到第一条跳转指令，
           这个时候我们就可以保证位于head_ensure及之前位置的指令，因中间没有跳转指令，一定会执行。因而，只要当head_ensure当前位置的指令是Store、Load指令，我们就可以向slbuffer发信息，增加head2。
           简单概括即对head2之前的Store/Load指令，我们根据判断出ROB中该条指令之前没有jump指令尚未执行，从而确定该条指令会被执行。

           2）同时SLBUFFER还需根据上个周期EX和SLBUFFER的计算结果遍历SLBUFFER进行数据的更新。

           3）此外，在我们的设计中，将SLBUFFER进行内存访问时计算需要访问的地址和对应的读取/存储内存的操作在SLBUFFER中一并实现，
           也可以考虑分成两个模块，该部分的实现只需判断队首的指令是否能执行并根据指令相应执行即可。

        2. 1）SLB每个周期会查看队头，若队头指令还未ready，则阻塞。

           2）当队头ready且是load指令时，SLB会直接执行load指令，包括计算地址和读内存，
           然后把结果通知ROB，同时将队头弹出。ROB commit到这条指令时通知Regfile写寄存器。

           3）当队头ready且是store指令时，SLB会等待ROB的commit，commit之后会SLB执行这
           条store指令，包括计算地址和写内存，写完后将队头弹出。
        */
    }

    void run_rob(){
        /*
        在这一部分你需要完成的工作：
        1. 实现一个先进先出的ROB，存储所有指令
        1. 根据issue阶段发射的指令信息分配空间进行存储。
        2. 根据EX阶段和SLBUFFER的计算得到的结果，遍历ROB，更新ROB中的值
        3. 对于队首的指令，如果已经完成计算及更新，进行commit
        */
    }

    void run_commit(){
        /*
        在这一部分你需要完成的工作：
        1. 根据ROB发出的信息更新寄存器的值，包括对应的ROB和是否被占用状态（注意考虑issue和commit同一个寄存器的情况）
        2. 遇到跳转指令更新pc值，并发出信号清空所有部分的信息存储（这条对于很多部分都有影响，需要慎重考虑）
        */
    }

    void update(){
        /*
        在这一部分你需要完成的工作：
        对于模拟中未完成同步的变量（即同时需记下两个周期的新/旧信息的变量）,进行数据更新。
        */
    }
    ~simulator(){}
};

#endif