`timescale 0.5ns/1ps

module Backend(
    input clk_i,
    input [31:0] pc_first_i,
    input [31:0] pc_second_i,
    output clk_o,
    output [31:0] pc_first_o,
    output [31:0] pc_second_o
);

//frequency_multiplier fm_u (
//    .clk(clk),
//    .clk_out(clk_o)
//);

assign clk_o = clk_i;
assign pc_first_o = pc_first_i;
assign pc_second_o = pc_second_i;

endmodule