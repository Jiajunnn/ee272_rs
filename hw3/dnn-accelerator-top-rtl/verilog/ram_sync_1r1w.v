module ram_sync_1r1w
#(
  parameter DATA_WIDTH = 8,
  parameter ADDR_WIDTH = 7,
  parameter DEPTH = 128
)(
  input clk,
  input wen,
  input [ADDR_WIDTH - 1 : 0] wadr,
  input [DATA_WIDTH - 1 : 0] wdata,
  input ren,
  input [ADDR_WIDTH - 1 : 0] radr,
  output [DATA_WIDTH - 1 : 0] rdata
);
  
  // synopsys translate_off
  reg [DATA_WIDTH - 1 : 0] rdata_reg;
  
  reg [DATA_WIDTH - 1 : 0] mem [DEPTH - 1 : 0];
  
  always @(posedge clk) begin
    if (wen) begin
      mem[wadr] <= wdata; // write port
      $write("Gainsight %m Write to address %d at time stamp: %d ns with data %h \n", wadr, $realtime, wdata);
    end
    if (ren) begin
      rdata_reg <= mem[radr]; // read port
      $write("Gainsight %m Read to address %d at time stamp: %d ns with data %h \n", radr, $realtime, rdata_reg);
    end
  end
  // synopsys translate_on

  assign rdata = rdata_reg;

endmodule
