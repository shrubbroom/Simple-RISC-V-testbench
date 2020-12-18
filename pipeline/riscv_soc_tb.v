`include "riscv.v"
`include "inst_mem.v"
`include "data_mem.v"

`timescale 1ps/1ps

module riscv_soc_tb();

   reg     clk;
   reg     rst;
   integer i;
   integer handle;


   initial begin
      clk = 1'b0;
      forever #50 clk = ~clk;
   end

   initial begin
      $dumpfile("riscv_soc_tb.vcd");
      $dumpvars(0, riscv_soc_tb);
      for(i = 0; i <= 31; i = i + 1)
        $dumpvars(0, riscv_soc_tb.riscv0.RF.register_file[i]);
      for(i = 0; i <= 3; i = i + 1)
        $dumpvars(0, riscv_soc_tb.data_mem0.data[i]);
      rst = 1'b1;
      #300    rst= 1'b0;
      #5000000;
      handle = $fopen("./data_mem_verilog.txt","w");
      for (i = 0; i <= 1023; i = i+1) begin
         $fwrite(handle, "%h\n", riscv_soc_tb.data_mem0.data[i]);
      end
      $fclose(handle);
      handle = $fopen("./register_file_verilog.txt", "w");
      $fwrite(handle, "%h\n", 32'b0);
      for (i = 1; i <= 31; i = i+1) begin
         $fwrite(handle, "%h\n", riscv_soc_tb.riscv0.RF.register_file[i]);
      end
      $fclose(handle);
      #1000 $finish;
      // #1000   $stop;
   end

   wire[31:0] inst_addr;
   wire [31:0] inst;
   wire        inst_ce;

   wire        data_ce;
   wire        data_we;
   wire [31:0] data_addr;
   wire [31:0] wdata;
   wire [31:0] rdata;
   wire [31:0] verify;


   riscv riscv0(
                .clk(clk),
                .rst(rst),

                .inst_addr_o(inst_addr),
                .inst_i(inst),
                .inst_ce_o(inst_ce),

                .data_ce_o(data_ce),
                .data_we_o(data_we),
                .data_addr_o(data_addr),
                .data_i(rdata),
                .data_o(wdata)
                );

   inst_mem inst_mem0(
                      .ce(inst_ce),
                      .addr(inst_addr),
                      .inst(inst)
                      );

   data_mem data_mem0(
                      .clk(clk),
                      .ce(data_ce),
                      .we(data_we),
                      .addr(data_addr),
                      .data_i(wdata),
                      .data_o(rdata),
                      .verify(verify)
                      );

endmodule
