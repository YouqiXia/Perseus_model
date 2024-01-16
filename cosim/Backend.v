module Backend(
    input [31:0] pc_first_i,
    input [31:0] pc_second_i,
    output [31:0] pc_first_o,
    output [31:0] pc_second_o
);

assign pc_first_o = pc_first_i;
assign pc_second_o = pc_second_i;

endmodule