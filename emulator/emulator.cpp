#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
using namespace std;
#define ADD 1
#define SUB 2
#define ADDI 3
#define AND 4
#define OR 5
#define XOR 6
#define BLT 7
#define BEQ 8
#define JAL 9
#define SLL 10
#define SRL 11
#define LW 12
#define SW 13
#define OP 0
#define RD 1
#define RS1 2
#define RS2 3
#define IMME 4

#define ALU_OP_ADD 0
#define ALU_OP_SUB 1
#define ALU_OP_AND 2
#define ALU_OP_OR 3
#define ALU_OP_XOR 4
#define ALU_OP_LT 5
#define ALU_OP_NONE 6
#define ALU_OP_SHIFT_LEFT 7
#define ALU_OP_SHIFT_RIGHT 8

#ifdef _WIN32
#define LB 2
#elif _WIN64
#define LB 2
#else
#define LB 1
#endif

void RISC_V_parser(signed int *, int **);
void RISC_V_arithmetic_parser(signed int *instruction, signed int **descriptor);
void RISC_V_imme_arithmetic_parser(signed int *instruction,
                                   signed int **descriptor);
void RISC_V_conditional_jmp_parser(signed int *instruction,
                                   signed int **descriptor);
void RISC_V_unconditional_jmp_parser(signed int *instruction,
                                     signed int **descriptor);
void RISC_V_load_parser(signed int *instruction, signed int **descriptor);
void RISC_V_store_parser(signed int *instruction, signed int **descriptor);
void RISC_V_add(signed int *, signed int *, signed int *);
void RISC_V_sub(signed int *, signed int *, signed int *);
void RISC_V_addi(signed int *, signed int *, signed int);
void RISC_V_and(signed int *, signed int *, signed int *);
void RISC_V_or(signed int *, signed int *, signed int *);
void RISC_V_xor(signed int *, signed int *, signed int *);
void RISC_V_blt(unsigned int *, signed int *, signed int *, signed int);
void RISC_V_beq(unsigned int *, signed int *, signed int *, signed int);
void RISC_V_jal(unsigned int *, signed int, signed int *);
void RISC_V_sll(signed int *, signed int *, signed int *);
void RISC_V_srl(signed int *, signed int *, signed int *);
void RISC_V_lw(signed int *, signed int, signed int *);
void RISC_V_sw(signed int *, signed int, signed int *);
signed int IO_inst_ram_read(unsigned int);
signed int IO_data_ram_read(signed int);
void IO_data_ram_write(signed int, signed int);
inline char IO_hex_to_char(int semibyte);
void DEBUG_print_all(signed int **RF, unsigned int *pc, signed int *instruction,
                     signed int **descriptor) {
  cerr << "Register files are:\n";
  for (int m = 0; m <= 7; ++m) {
    for (int i = 0; i <= 3; ++i) {
      cerr << "x" << setw(2) << left << i + m * 4 << " = " << setw(10)
           << *RF[i + m * 4] << " | ";
    }
    cerr << "\n";
  }
  cerr << "PC is: " << hex << (*pc) << dec << "\n";
  cerr << "Previous instruction is: " << bitset<32>(*instruction) << "\n";
  cerr << "Instruction type is: ";
  switch (*descriptor[OP]) {
  case ADD:
    cerr << "ADD"
         << " (" << bitset<4>(ALU_OP_ADD) << ")"
         << "\n";
    break;
  case SUB:
    cerr << "SUB"
         << " (" << bitset<4>(ALU_OP_SUB) << ")"
         << "\n";
    break;
  case ADDI:
    cerr << "ADDI"
         << " (" << bitset<4>(ALU_OP_ADD) << ")"
         << "\n";
    break;
  case AND:
    cerr << "AND"
         << " (" << bitset<4>(ALU_OP_AND) << ")"
         << "\n";
    break;
  case OR:
    cerr << "OR"
         << " (" << bitset<4>(ALU_OP_OR) << ")"
         << "\n";
    break;
  case XOR:
    cerr << "XOR"
         << " (" << bitset<4>(ALU_OP_XOR) << ")"
         << "\n";
    break;
  case BLT:
    cerr << "BLT"
         << " (" << bitset<4>(ALU_OP_LT) << ")"
         << "\n";
    break;
  case BEQ:
    cerr << "BEQ"
         << " (" << bitset<4>(ALU_OP_SUB) << ")"
         << "\n";
    break;
  case JAL:
    cerr << "JAL"
         << " (" << bitset<4>(ALU_OP_NONE) << ")"
         << "\n";
    break;
  case SLL:
    cerr << "SLL"
         << " (" << bitset<4>(ALU_OP_SHIFT_LEFT) << ")"
         << "\n";
    break;
  case SRL:
    cerr << "SRL"
         << " (" << bitset<4>(ALU_OP_SHIFT_RIGHT) << ")"
         << "\n";
    break;
  case LW:
    cerr << "LW"
         << " (" << bitset<4>(ALU_OP_ADD) << ")"
         << "\n";
    break;
  case SW:
    cerr << "SW"
         << " (" << bitset<4>(ALU_OP_ADD) << ")"
         << "\n";
    break;
  default:
    cerr << "Undetermined" << '\n';
  }

  cerr << "RS1 is: x" << *descriptor[RS1] << " = "
       << bitset<32>(*RF[*descriptor[RS1]]) << " (" << dec
       << *RF[*descriptor[RS1]] << ")"
       << "\n";
  cerr << "RS2 is: x" << *descriptor[RS2] << " = "
       << bitset<32>(*RF[*descriptor[RS2]]) << " (" << dec
       << *RF[*descriptor[RS2]] << ")"
       << "\n";
  cerr << "RD is: x" << *descriptor[RD] << " = "
       << bitset<32>(*RF[*descriptor[RD]]) << " (" << dec
       << *RF[*descriptor[RD]] << ")"
       << "\n";
  cerr << "IMME is: " << bitset<32>(*descriptor[IMME]) << " ("
       << *descriptor[IMME] << ")"
       << "\n";
}
FILE *IO_inst_ram_normalize(FILE *);
FILE *inst_ram;
FILE *data_ram;
int main(int argc, char *argv[]) {
  signed int *Register_File[32];
  int Register_File_Used[32];
  unsigned int *pc;
  signed int *instruction;
  signed int *descriptor[5];

  if (argc == 1) {
    cout << "Allocating register file, PC and instruction , the size of each "
            "register is "
         << sizeof(signed int) << " bytes\n";
    for (int i = 0; i <= 31; ++i) {
      Register_File[i] = (signed int *)malloc(sizeof(signed int));
      *Register_File[i] = 0;
    }
    pc = (unsigned int *)malloc(sizeof(unsigned int));
    *(pc) = 0;

    instruction = (signed int *)malloc(sizeof(signed int));
    *instruction = 0;

    for (int i = 0; i <= 4; ++i) {
      descriptor[i] = (signed int *)malloc(sizeof(signed int));
      *descriptor[i] = 0;
    }
    cout << "Connecting instruction RAM file and data RAM file\n";
    inst_ram = fopen("machinecode.txt", "r");
    inst_ram = IO_inst_ram_normalize(inst_ram);
    data_ram = fopen("data_mem.txt", "r+");

    char command;
    int print_flag = 0;
    cout << "Emulator is kicked up\n";
    while (true) {
      while (true) {
        cout << "Type instruction (n, next) (p, print) (t, toggle print on "
                "each step):\n";
        cin >> command;
        if (command == 'n')
          break;
        else
          switch (command) {
          case 'p':
            DEBUG_print_all(Register_File, pc, instruction, descriptor);
            break;
          case 't':
            print_flag = (print_flag == 0);
            if (print_flag)
              cout << "Turn on printing in each step\n";
            else
              cout << "Turn off printing in each step\n";
            break;
          default:
            cout << "This is not a command " << command << '\n';
          }
      }

      *instruction = IO_inst_ram_read(*pc);
      RISC_V_parser(instruction, descriptor);
      switch (*descriptor[OP]) {
      case ADD:
        RISC_V_add(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case SUB:
        RISC_V_sub(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case ADDI:
        RISC_V_addi(Register_File[*descriptor[RD]],
                    Register_File[*descriptor[RS1]], *descriptor[IMME]);
        if (Register_File_Used[*descriptor[RS1]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case AND:
        RISC_V_and(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case OR:
        RISC_V_or(Register_File[*descriptor[RD]],
                  Register_File[*descriptor[RS1]],
                  Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;

        break;
      case XOR:
        RISC_V_xor(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;

        break;
      case BLT:
        if (Register_File_Used[*descriptor[RS1]] && Register_File_Used[*descriptor[RS2]])
          RISC_V_blt(pc, Register_File[*descriptor[RS1]],
                     Register_File[*descriptor[RS2]], *descriptor[IMME]);
        else {
          cerr << "Assembly wrong, jmp with undefined register" << '\n';
          DEBUG_print_all(Register_File, pc, instruction, descriptor);
          exit(1);
        }
        break;
      case BEQ:
        if (Register_File_Used[*descriptor[RS1]] && Register_File_Used[*descriptor[RS2]])
          RISC_V_beq(pc, Register_File[*descriptor[RS1]],
                     Register_File[*descriptor[RS2]], *descriptor[IMME]);
        else {
          cerr << "Assembly wrong, jmp with undefined register" << '\n';
          DEBUG_print_all(Register_File, pc, instruction, descriptor);
          exit(1);
        }
        break;
      case JAL:
        RISC_V_jal(pc, *descriptor[IMME], Register_File[*descriptor[RD]]);
        Register_File_Used[RD] = 1;
        break;
      case SLL:
        RISC_V_sll(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case SRL:
        RISC_V_srl(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case LW:
        if (Register_File_Used[*descriptor[RS1]]) {
          RISC_V_lw(Register_File[*descriptor[RD]], *descriptor[IMME],
                    Register_File[*descriptor[RS1]]);
          Register_File_Used[*descriptor[RD]] = 1;
        } else {
          cerr << "Assembly wrong, load with undefined register" << '\n';
          DEBUG_print_all(Register_File, pc, instruction, descriptor);
          exit(1);
        }

        break;
      case SW:
        if (Register_File_Used[*descriptor[RS1]]) {
          RISC_V_sw(Register_File[*descriptor[RS2]], *descriptor[IMME],
                    Register_File[*descriptor[RS1]]);
        } else {
          cerr << "Assembly wrong, load with undefined register" << '\n';
          DEBUG_print_all(Register_File, pc, instruction, descriptor);
          exit(1);
        }
        break;
      }

      *Register_File[0] = 0;
      if (*descriptor[OP] != BEQ && *descriptor[OP] != BLT &&
          *descriptor[OP] != JAL)
        *(pc) = *(pc) + 4;
      if (print_flag)
        DEBUG_print_all(Register_File, pc, instruction, descriptor);
    }
  } else {
    for (int i = 0; i <= 31; ++i) {
      Register_File[i] = (signed int *)malloc(sizeof(signed int));
      *Register_File[i] = 0;
      Register_File_Used[i] = 0;
    }
    Register_File_Used[0] = 1;
    pc = (unsigned int *)malloc(sizeof(unsigned int));
    *(pc) = 0;
    instruction = (signed int *)malloc(sizeof(signed int));
    *instruction = 0;
    for (int i = 0; i <= 4; ++i) {
      descriptor[i] = (int *)malloc(sizeof(int));
      *descriptor[i] = 0;
    }
    inst_ram = fopen("machinecode.txt", "r");
    inst_ram = IO_inst_ram_normalize(inst_ram);
    data_ram = fopen("data_mem_emu.txt", "r+");

    char command;
    int print_flag = 0;
    while (true) {
      try {
        *instruction = IO_inst_ram_read(*pc);
      } catch (int x) {
        if (x == 100) {
          FILE *RF = fopen("register_file_emu.txt", "w");
          for (int i = 0; i <= 31; ++i) {
            if (Register_File_Used[i]) {
              fputc(IO_hex_to_char((((unsigned int)*Register_File[i]) >> 28) &
                                   0b1111),
                    RF);
              fputc(IO_hex_to_char((((unsigned int)*Register_File[i]) >> 24) &
                                   0b1111),
                    RF);
              fputc(IO_hex_to_char((((unsigned int)*Register_File[i]) >> 20) &
                                   0b1111),
                    RF);
              fputc(IO_hex_to_char((((unsigned int)*Register_File[i]) >> 16) &
                                   0b1111),
                    RF);
              fputc(IO_hex_to_char((((unsigned int)*Register_File[i]) >> 12) &
                                   0b1111),
                    RF);
              fputc(IO_hex_to_char((((unsigned int)*Register_File[i]) >> 8) &
                                   0b1111),
                    RF);
              fputc(IO_hex_to_char((((unsigned int)*Register_File[i]) >> 4) &
                                   0b1111),
                    RF);
              fputc(IO_hex_to_char((((unsigned int)*Register_File[i]) >> 0) &
                                   0b1111),
                    RF);
              fputc('\n', RF);
            } else {
              for (int j = 1; j <= 8; ++j) {
                fputc('x', RF);
              }
              fputc('\n', RF);
            }
          }
          fclose(RF);
          fclose(inst_ram);
          fclose(data_ram);
          return (0);
        } else
          return (1);
      }
      RISC_V_parser(instruction, descriptor);
      Register_File_Used[*descriptor[RD]] = 1;
      switch (*descriptor[OP]) {
      case ADD:
        RISC_V_add(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case SUB:
        RISC_V_sub(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case ADDI:
        RISC_V_addi(Register_File[*descriptor[RD]],
                    Register_File[*descriptor[RS1]], *descriptor[IMME]);
        if (Register_File_Used[*descriptor[RS1]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case AND:
        RISC_V_and(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case OR:
        RISC_V_or(Register_File[*descriptor[RD]],
                  Register_File[*descriptor[RS1]],
                  Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;

        break;
      case XOR:
        RISC_V_xor(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;

        break;
      case BLT:
        if (Register_File_Used[*descriptor[RS1]] && Register_File_Used[*descriptor[RS2]])
          RISC_V_blt(pc, Register_File[*descriptor[RS1]],
                     Register_File[*descriptor[RS2]], *descriptor[IMME]);
        else {
          cerr << "Assembly wrong, jmp with undefined register" << '\n';
          DEBUG_print_all(Register_File, pc, instruction, descriptor);
          exit(1);
        }
        break;
      case BEQ:
        if (Register_File_Used[*descriptor[RS1]] && Register_File_Used[*descriptor[RS2]])
          RISC_V_beq(pc, Register_File[*descriptor[RS1]],
                     Register_File[*descriptor[RS2]], *descriptor[IMME]);
        else {
          cerr << "Assembly wrong, jmp with undefined register" << '\n';
          DEBUG_print_all(Register_File, pc, instruction, descriptor);
          exit(1);
        }
        break;
      case JAL:
        RISC_V_jal(pc, *descriptor[IMME], Register_File[*descriptor[RD]]);
        Register_File_Used[RD] = 1;
        break;
      case SLL:
        RISC_V_sll(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case SRL:
        RISC_V_srl(Register_File[*descriptor[RD]],
                   Register_File[*descriptor[RS1]],
                   Register_File[*descriptor[RS2]]);
        if (Register_File_Used[*descriptor[RS1]] &&
            Register_File_Used[*descriptor[RS2]])
          Register_File_Used[*descriptor[RD]] = 1;
        break;
      case LW:
        if (Register_File_Used[*descriptor[RS1]]) {
          RISC_V_lw(Register_File[*descriptor[RD]], *descriptor[IMME],
                    Register_File[*descriptor[RS1]]);
          Register_File_Used[*descriptor[RD]] = 1;
        } else {
          cerr << "Assembly wrong, load with undefined register" << '\n';
          DEBUG_print_all(Register_File, pc, instruction, descriptor);
          exit(1);
        }

        break;
      case SW:
        if (Register_File_Used[*descriptor[RS1]]) {
          RISC_V_sw(Register_File[*descriptor[RS2]], *descriptor[IMME],
                    Register_File[*descriptor[RS1]]);
        } else {
          cerr << "Assembly wrong, load with undefined register" << '\n';
          DEBUG_print_all(Register_File, pc, instruction, descriptor);
          exit(1);
        }
        break;
      }
      *Register_File[0] = 0;
      if (*descriptor[OP] != BEQ && *descriptor[OP] != BLT &&
          *descriptor[OP] != JAL)
        *(pc) = *(pc) + 4;
      // DEBUG_print_all(Register_File, pc, instruction, descriptor);
    }
  }
}

void RISC_V_parser(signed int *instruction, signed int **descriptor) {
  switch (*instruction & 0b1111111) {
  case 0b0110011:
    RISC_V_arithmetic_parser(instruction, descriptor);
    break;
  case 0b0010011:
    RISC_V_imme_arithmetic_parser(instruction, descriptor);
    break;
  case 0b1100011:
    RISC_V_conditional_jmp_parser(instruction, descriptor);
    break;
  case 0b1101111:
    RISC_V_unconditional_jmp_parser(instruction, descriptor);
    break;
  case 0b0000011:
    RISC_V_load_parser(instruction, descriptor);
    break;
  case 0b0100011:
    RISC_V_store_parser(instruction, descriptor);
    break;
  default:
    cerr << "fatal error in opcode parse, opcode: "<<bitset<7> (*instruction & 0b1111111)<<'\n';
    exit(1);
  }
}
void RISC_V_arithmetic_parser(signed int *instruction, int **descriptor) {
  *descriptor[RS1] = (*instruction >> 15) & 0b11111;
  *descriptor[RS2] = (*instruction >> 20) & 0b11111;
  *descriptor[RD] = (*instruction >> 7) & 0b11111;
  switch ((*instruction >> 12) & 0b111) {
  case 0b000: {
    if ((*instruction & 0xF0000000) == 0) {
      *descriptor[OP] = ADD;
      break;
    } else {
      *descriptor[OP] = SUB;
      break;
    }
  }
  case 0b001: {
    *descriptor[OP] = SLL;
    break;
  }
  case 0b101: {
    *descriptor[OP] = SRL;
    break;
  }
  case 0b100: {
    *descriptor[OP] = XOR;
    break;
  }
  case 0b110: {
    *descriptor[OP] = OR;
    break;
  }
  case 0b111: {
    *descriptor[OP] = AND;
    break;
  }
  default:
    cerr << "fatal error in func3 parse";
    exit(1);
  }
}
void RISC_V_imme_arithmetic_parser(signed int *instruction,
                                   signed int **descriptor) {
  if (((*instruction >> 12) & 0b111) == 0b000) {
    *descriptor[OP] = ADDI;
    *descriptor[RD] = (*instruction >> 7) & 0b11111;
    *descriptor[RS1] = (*instruction >> 15) & 0b11111;
    *descriptor[IMME] = (*instruction >> 20);
  }
}

void RISC_V_conditional_jmp_parser(signed int *instruction,
                                   signed int **descriptor) {
  *descriptor[RS1] = (*instruction >> 15) & 0b11111;
  *descriptor[RS2] = (*instruction >> 20) & 0b11111;
  *descriptor[IMME] = ((*instruction >> 7) & 0b11110) |
                      ((*instruction >> 20) & 0b11111100000) |
                      ((*instruction << 4) & 0b100000000000) |
                      ((*instruction >> 19) & 0xfffff000);
  switch ((*instruction >> 12) & 0b111) {
  case 0b100:
    *descriptor[OP] = BLT;
    break;
  case 0b000:
    *descriptor[OP] = BEQ;
    break;
  }
}
void RISC_V_unconditional_jmp_parser(signed int *instruction,
                                     signed int **descriptor) {
  *descriptor[OP] = JAL;
  *descriptor[RD] = ((*instruction) >> 7) & 0b11111;
  *descriptor[IMME] = ((*instruction >> 20) & 0b11111111110) |
                      ((*instruction >> 9) & 0b100000000000) |
                      ((*instruction) & 0xff000) |
                      ((*instruction >> 11) & 0xfff00000);
}
void RISC_V_load_parser(signed int *instruction, signed int **descriptor) {
  *descriptor[OP] = LW;
  *descriptor[RD] = ((*instruction) >> 7) & 0b11111;
  *descriptor[RS1] = ((*instruction) >> 15) & 0b11111;
  *descriptor[IMME] = ((*instruction) >> 20);
}
void RISC_V_store_parser(signed int *instruction, signed int **descriptor) {
  *descriptor[OP] = SW;
  *descriptor[RS1] = ((*instruction) >> 15) & 0b11111;
  *descriptor[RS2] = ((*instruction) >> 20) & 0b11111;
  *descriptor[IMME] =
      ((*instruction >> 7) & 0b11111) | ((*instruction >> 20) & 0xffffffe0);
}
void RISC_V_add(signed int *rd, signed int *rs1, signed int *rs2) {
  *(rd) = *(rs1) + *(rs2);
}
void RISC_V_sub(signed int *rd, signed int *rs1, signed int *rs2) {
  *(rd) = *(rs1) - *(rs2);
}
void RISC_V_addi(signed int *rd, signed int *rs1, signed int imme) {
  *(rd) = *(rs1) + imme;
}
void RISC_V_and(signed int *rd, signed int *rs1, signed int *rs2) {
  *(rd) = *(rs1) & *(rs2);
}
void RISC_V_or(signed int *rd, signed int *rs1, signed int *rs2) {
  *(rd) = *(rs1) | *(rs2);
}
void RISC_V_xor(signed int *rd, signed int *rs1, signed int *rs2) {
  *(rd) = *(rs1) ^ *(rs2);
}
void RISC_V_blt(unsigned int *pc, signed int *rs1, signed int *rs2,
                signed int imme) {
  if (*(rs1) < *(rs2))
    *(pc) = *(pc) + (unsigned int)imme;
  else
    *(pc) = *(pc) + 4;
}
void RISC_V_beq(unsigned int *pc, signed int *rs1, signed int *rs2,
                signed int imme) {
  if (*(rs1) == *(rs2))
    *(pc) = *(pc) + (unsigned int)imme;
  else
    *(pc) = *(pc) + 4;
}
void RISC_V_jal(unsigned int *pc, signed int imme, signed int *rd) {
  *(rd) = *(pc) + 4;
  *(pc) = *(pc) + (unsigned int)imme;
}

void RISC_V_sll(signed int *rd, signed int *rs1, signed int *rs2) {
  *(rd) = ((unsigned int)*(rs1)) << ((unsigned int)(*rs2 & 0b11111));
}

void RISC_V_srl(signed int *rd, signed int *rs1, signed int *rs2) {
  *(rd) = ((unsigned int)*(rs1)) >> ((unsigned int)(*rs2 & 0b11111));
}

void RISC_V_lw(signed int *dest, signed int offset, signed int *base) {
  *(dest) = IO_data_ram_read(*(base) + offset);
}

void RISC_V_sw(signed int *src, signed int offset, signed int *base) {
  IO_data_ram_write(*src, *base + offset);
}

FILE *IO_inst_ram_normalize(FILE *prev) {
  FILE *normal_inst_ram = fopen("machinecode_normal.txt", "w");
  int tmp;
  while ((tmp = fgetc(prev)) != EOF) {
    if (tmp == 0x20 || tmp == 0x2f || tmp == 0x0d) {
    } else {
      fputc(tmp, normal_inst_ram);
    }
  }
  fclose(prev);
  fclose(normal_inst_ram);
  return fopen("machinecode_normal.txt", "r");
}

inline unsigned int IO_char_to_hex(char semibyte) {
  if (semibyte >= '0' && semibyte <= '9')
    return semibyte - 48;
  if (semibyte >= 'a' && semibyte <= 'f')
    return semibyte + 10 - 97;
  else {
    cout << "fatal error in converting char to hex\n";
    cout << "converting " << semibyte << '\n';
    throw 100;
    // exit(0);
  }
}

inline char IO_hex_to_char(int semibyte) {
  if (semibyte >= 0 && semibyte <= 9)
    return semibyte + 48;
  if (semibyte >= 10 && semibyte <= 15)
    return semibyte - 10 + 97;
  else {
    cerr << "fatal error in converting hex to char\n";
    cerr << "converting " << semibyte << '\n';
    exit(0);
  }
}
signed int IO_inst_ram_read(unsigned int pc) {
  int instruction = 0;
  int semibyte;
  semibyte = fgetc(inst_ram);
  if (fseek(inst_ram, (pc / 4) * (8 + LB), SEEK_SET)) {
    cerr << "exiting in seeking instruction, PC is " << pc;
    exit(1);
  }
  for (int i = 0; i <= 7; ++i) {
    semibyte = fgetc(inst_ram);
    instruction = (instruction << 4) + IO_char_to_hex(semibyte);
  }
  return instruction;
}
signed int IO_data_ram_read(signed int addr) {
  signed int value = 0;
  signed int semibyte;
  signed int tmp;
  int counter = 0;
  if (fseek(data_ram, addr * (2 + LB), SEEK_SET)) {
    cerr << "fatal error in seeking data position, seeking: "<< addr << "\n";
    exit(1);
  }
  for (int i = 0; i <= 3; ++i) {
    semibyte = fgetc(data_ram);
    value = value + (IO_char_to_hex(semibyte) << ((counter + 1) * 4));
    counter++;
    semibyte = fgetc(data_ram);
    value = value + (IO_char_to_hex(semibyte) << ((counter - 1) * 4));
    counter++;
    if (LB == 2)
      semibyte = fgetc(data_ram);
    semibyte = fgetc(data_ram);
  }
  return value;
}

void IO_data_ram_write(signed int src_value, signed int addr) {
  signed tmp;
  if (fseek(data_ram, addr * (2 + LB), SEEK_SET)) {
    cerr << "fatal error in seeking data position\n";
    exit(1);
  }
  for (int i = 0; i <= 3; ++i) {
    tmp = IO_hex_to_char(src_value & 0xf);
    src_value = src_value >> 4;
    fputc(IO_hex_to_char(src_value & 0xf), data_ram);
    fputc(tmp, data_ram);
    src_value = src_value >> 4;
    if (LB == 2)
      fputc(0xd, data_ram);
    fputc(0xa, data_ram);
  }
}
