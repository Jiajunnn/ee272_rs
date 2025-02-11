// Write a directed test for the ifmap double buffer module. Make sure you test 
// all its ports and its behaviour when your switch banks.

// Your code starts here
`define ARRAY_HEIGHT 4
`define IC1 2
`define IY0 5
`define IX0 5
`define IFMAP_DATA_WIDTH 16

module ifmap_double_buffer_tb;
    parameter DATA_WIDTH = `ARRAY_HEIGHT * `IFMAP_DATA_WIDTH;
    parameter BANK_DEPTH = `IC1 * `IX0 * `IY0 * 2;
    parameter BANK_ADDR_WIDTH = $clog2(BANK_DEPTH);

	reg clk;
	reg rst_n;
 	reg switch_banks;
	reg ren;
	reg [BANK_ADDR_WIDTH-1:0] radr;
	reg [DATA_WIDTH-1:0] rdata;
	reg wen;
	reg [BANK_ADDR_WIDTH-1:0] wadr;
	reg [DATA_WIDTH-1:0] wdata;


	always #10 clk = ~clk;

	double_buffer #(
		.DATA_WIDTH(DATA_WIDTH),
		.BANK_ADDR_WIDTH(BANK_ADDR_WIDTH),
		.BANK_DEPTH(BANK_DEPTH)
	) double_buffer_inst (
		.clk(clk),
		.rst_n(rst_n),
		.switch_banks(switch_banks),
		.ren(ren),
		.radr(radr),
		.rdata(rdata),
		.wen(wen),
		.wadr(wadr),
		.wdata(wdata)
	);	

	initial begin
		clk <= 0;
		rst_n <= 1;
		switch_banks <= 0;
		ren <= 0;
		radr <= 0;
		wen <= 0;
		wadr <= 0;
		wdata <= 0;
		
		// Reset block
		#20 rst_n <= 0;
		#20 rst_n <= 1;
		
		// Write to Bank 1
		wen <= 1;
		wdata <= 64'hdeadbeef;
		wadr <= 0;
		#20;
		wdata <= 64'habcdabcd;
		wadr <= 1;
		#20;
		wen <= 0;
		#20;
		
		// Switch banks and read what I wrote
		switch_banks <= 1;
		#20; 
		switch_banks <= 0; 
		#20; 
		radr <= 0;
		ren <= 1;
		#20; 
		assert(rdata == 64'hdeadbeef);
		radr <= 1;
		ren <= 1;
		#20; 
		assert(rdata == 64'habcdabcd);
		
		// Write something to Bank 0 including last address
		wen <= 1;
		wdata <= 64'hfefefefe;
		wadr <= 5;
		#20;
		wdata <= 64'hbaddadda;
		wadr <= BANK_ADDR_WIDTH-1;
		#20;
		wen <= 0;
		#20;

		// Switch banks and read what I wrote
		switch_banks <= 1;
		#20; 
		switch_banks <= 0; 
		#20; 
		radr <= 5;
		ren <= 1;
		#20; 
		assert(rdata == 64'hfefefefe);
		radr <= BANK_ADDR_WIDTH-1;
		ren <= 1;
		#20; 
		assert(rdata == 64'hbaddadda);

		//Switch banks read first value then reset to go back to bank 0
		switch_banks <= 1;
		#20; 
		switch_banks <= 0; 
		#20; 
		radr <= 0;
		ren <= 1;
		#20; 
		assert(rdata == 64'hdeadbeef);
		#20; 
		rst_n <= 0;
		#20 rst_n <= 1;
		#20; 
		radr <= 5;
		ren <= 1;
		#20; 
		assert(rdata == 64'hfefefefe);


		// Finally read and write same cycle (and read the value
		// I wrote)
		radr <= BANK_ADDR_WIDTH-1;
		ren <= 1;
		wen <= 1;
		wdata <= 64'h12345678;
		wadr <= 5;
		#20; 
		assert(rdata == 64'hbaddadda);
		switch_banks <= 1;
		#20; 
		switch_banks <= 0; 
		#20; 	
		radr <= 5;
		ren <= 1;
		#20; 
		assert(rdata == 64'h12345678);

	end

   	initial begin
    		$vcdplusfile("dump.vcd");
    		$vcdplusmemon();
    		$vcdpluson(0, ifmap_double_buffer_tb);
			`ifdef FSDB
			$fsdbDumpfile("dump.fsdb");
    		$fsdbDumpvars(0, ifmap_double_buffer_tb);
    		$fsdbDumpMDA();
			`endif
    		#2000;
    		$finish(2);
  	end

endmodule

// Your code ends here
