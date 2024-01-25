module select2_unused #(
    parameter RS_SIZE = 4,
    parameter RS_INDEX_WIDTH = 2
)(
    input logic [RS_SIZE-1:0] rs_unused_i,
    output logic rs_first_write_ready_o,
    output logic rs_second_write_ready_o,
    output logic [RS_INDEX_WIDTH-1:0] wr_rs_index_first_o,
    output logic [RS_INDEX_WIDTH-1:0] wr_rs_index_second_o
);

    reg rs_first_write_ready;
    reg rs_second_write_ready;

    always @* begin
        rs_first_write_ready = 0;
        rs_second_write_ready = 0;
        
        for (int i = 0; i < RS_SIZE; i = i + 1) begin
            if (rs_unused_i[i]) begin
                if (!rs_first_write_ready) begin
                    rs_first_write_ready = 1;
                    wr_rs_index_first_o = i[RS_INDEX_WIDTH-1:0];
                end
                else if (!rs_second_write_ready) begin
                    rs_second_write_ready = 1;
                    wr_rs_index_second_o = i[RS_INDEX_WIDTH-1:0];
                    break;
                end
            end
        end
    end

    // Assign internal signals to output signals
    assign rs_first_write_ready_o = rs_first_write_ready;
    assign rs_second_write_ready_o = rs_second_write_ready;

endmodule