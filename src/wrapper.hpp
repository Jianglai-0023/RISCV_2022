#ifndef RISC_V_WRAPPER_HPP
#define RISC_V_WRAPPER_HPP

class Wrapper {
    using u32 = unsigned int;
    using u8 = char;
    u8 Memory[1048576];
    u32 Register[32];
public:
    void parser(std::istream &Input) {
        while (!Input.fail()) {
            std::string s;
            char *p;
            Input >> s;
            u32 point;
            if (s[0] == '@') {
                point = strtoul(s.substr(1, 8).c_str(), &p, 16);
            } else {
                char *p;
                Memory[point] = strtoul(s.c_str(), &p, 16);
                point++;
            }
        }
    }

};

#endif //RISC_V_WRAPPER_HPP