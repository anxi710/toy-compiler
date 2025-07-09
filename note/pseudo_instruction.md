# RISC-V 伪指令 vs 真实指令对照表

| 伪指令             | 实际展开指令                                  | 说明                            |
| --------------- | --------------------------------------- | ----------------------------- |
| `j label`       | `jal x0, label`                         | 无条件跳转，无返回                     |
| `jr rs`         | `jalr x0, 0(rs)`                        | 无条件跳转到寄存器 `rs` 指定地址           |
| `ret`           | `jalr x0, 0(ra)`                        | 返回到 `ra` 保存的地址                |
| `call label`    | `auipc ra, offset; jalr ra, offset(ra)` | 跨段调用（用于链接器处理远跳转）              |
| `tail label`    | `auipc x6, offset; jalr x0, offset(x6)` | 跨段跳转无返回（尾调用优化）                |
| `nop`           | `addi x0, x0, 0`                        | 空操作指令                         |
| `mv rd, rs`     | `addi rd, rs, 0`                        | 拷贝寄存器内容                       |
| `li rd, imm`    | 多种指令组合（`addi`, `lui`, `ori`等）           | 加载立即数（汇编器自动分解为合适的指令）          |
| `la rd, symbol` | `auipc rd, offset; addi rd, rd, offset` | 加载地址                          |
| `call symbol`   | `auipc ra, offset; jalr ra, offset(ra)` | 带返回地址调用函数                     |
| `tail symbol`   | `auipc t1, offset; jalr x0, offset(t1)` | 跳转到函数，无返回                     |
| `sext.w rd, rs` | `addiw rd, rs, 0`                       | 有符号扩展 `rs` 到 64 位寄存器（RV64 专用） |
| `not rd, rs`    | `xori rd, rs, -1`                       | 对寄存器按位取反                      |
| `neg rd, rs`    | `sub rd, x0, rs`                        | 对寄存器求负                        |
| `snez rd, rs`   | `sltu rd, x0, rs`                       | 判断 `rs != 0`                  |
| `seqz rd, rs`   | `sltiu rd, rs, 1`                       | 判断 `rs == 0`                  |
