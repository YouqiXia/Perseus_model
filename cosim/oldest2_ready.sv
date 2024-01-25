module oldest2_ready #(
    parameter RS_SIZE = 4,
    parameter RS_INDEX_WIDTH = 2,
    parameter AGE_WIDTH = 16
)(
    input logic [RS_SIZE-1:0] ready_i,
    input logic [AGE_WIDTH-1:0] ages_i [RS_SIZE-1:0],
    output logic first_valid_o,
    output logic [RS_INDEX_WIDTH-1:0] first_index_o,
    output logic second_valid_o,
    output logic [RS_INDEX_WIDTH-1:0] second_index_o
);

    logic [AGE_WIDTH-1:0] oldest_age, second_oldest_age;
    logic [RS_INDEX_WIDTH:0] oldest_temp_index1, oldest_temp_index2;

    always_comb begin
        oldest_age = {AGE_WIDTH{1'b1}};
        second_oldest_age = {AGE_WIDTH{1'b1}};
        oldest_temp_index1 = RS_SIZE[RS_INDEX_WIDTH:0];
        oldest_temp_index2 = RS_SIZE[RS_INDEX_WIDTH:0];

        for (int i = 0; i < RS_SIZE; i++) begin
            if (ready_i[i]) begin
                if (ages_i[i] < oldest_age) begin
                    second_oldest_age = oldest_age;
                    oldest_temp_index2 = oldest_temp_index1;
                    oldest_age = ages_i[i];
                    oldest_temp_index1 = i[RS_INDEX_WIDTH:0];
                end else if (ages_i[i] < second_oldest_age) begin
                    second_oldest_age = ages_i[i];
                    oldest_temp_index2 = i[RS_INDEX_WIDTH:0];
                end
            end
        end

        first_index_o = oldest_temp_index1[RS_INDEX_WIDTH-1:0];
        second_index_o = oldest_temp_index2[RS_INDEX_WIDTH-1:0];
        first_valid_o = (oldest_temp_index1 == RS_SIZE[RS_INDEX_WIDTH:0]) ? 0 : 1'b1;
        second_valid_o = (oldest_temp_index2 == RS_SIZE[RS_INDEX_WIDTH:0]) ? 0 : 1'b1;
    end

endmodule
