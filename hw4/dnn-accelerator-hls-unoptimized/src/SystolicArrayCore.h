#ifndef SYSTOLIC_ARRAY_CORE_H
#define SYSTOLIC_ARRAY_CORE_H

#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/comparison/not_equal.hpp>
#include <boost/preprocessor/repetition/for.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>

#include "ProcessingElement.h"
#include "Fifo.h"

// Define this macro for debug logging
#define HLS_DEBUG 0
#if HLS_DEBUG
#ifndef __SYNTHESIS__
#include <iostream>
#include <fstream>
#include <string>

// Only works for square arrays
template <typename T>
void log_matrix(std::ofstream& file, T* data, int iteration, int side_length) {
    file << "Iteration: " << iteration << '\n';
    for (int r = 0; r < side_length; r++) {
        for (int c = 0; c < side_length; c++) {
            file << int(data[r][c].to_int()) << ' ';
        }
        file << '\n';
    }
    file << '\n';
}
#endif
#endif


struct LoopIndices{
    uint_16 ic1_idx;
    uint_16 fx_idx;
    uint_16 fy_idx;
};



template <typename IDTYPE, typename WDTYPE, typename ODTYPE, int OC0, int IC0>
class SystolicArrayCore
{
    #if HLS_DEBUG
    #ifndef __SYNTHESIS__
    // Create log file information
    std::ofstream input_file;
    std::ofstream weight_file;
    std::ofstream psum_file;
    #endif
    #endif


public:
    SystolicArrayCore() {
        #if HLS_DEBUG
        #ifndef __SYNTHESIS__

        // Creates filenames
        std::string input_filename = "input_file";
        std::string weight_filename = "weight_file";
        std::string psum_filename = "psum_file";

        // Opens log files when debugging
        input_file.open(input_filename.c_str());
        weight_file.open(weight_filename.c_str());
        psum_file.open(psum_filename.c_str());
        bool open_success = true;
        open_success = open_success && input_file.is_open();
        open_success = open_success && weight_file.is_open();
        open_success = open_success && psum_file.is_open();

        if (!open_success) {
            std::cerr << "Failed to open one or more log files." << std::endl;
        }
        #endif
        #endif
    }

#pragma hls_design interface
#pragma hls_pipeline_init_interval 1
    void CCS_BLOCK(run)(
        ac_channel<PackedInt<INPUT_PRECISION, IC0> > &input, 
        ac_channel<PackedInt<WEIGHT_PRECISION, OC0> > &weight, 
        ac_channel<PackedInt<OUTPUT_PRECISION, OC0> > &output,
        ac_channel<Params> &paramsIn,
        ac_channel<LoopIndices> &loopIndicesIn)
    {
        #ifndef __SYNTHESIS__
        // assert(params.OX0 * params.OY0 <= ACCUMULATION_BUFFER_SIZE);
        // Debug example:
        // printf("paramsIn channel size: %d\n", paramsIn.size());
        // printf("loopIndicesIn channel size: %d\n", loopIndicesIn.size());
        // printf("weight channel size: %d\n", weight.size());
        // printf("input channel size: %d\n\n", input.size());
        #endif

        #ifndef __SYNTHESIS__
        while(loopIndicesIn.available(1))
        #endif
        {
            // -------------------------------
            // Read in the params and loop indices from the channel
            // Your code starts here
            Params params = paramsIn.read();
            LoopIndices loopIndices = loopIndicesIn.read();
            // Your code ends here
            // -------------------------------
            // int step_bound = params.OY0 * params.OX0 + params.FY + params.FX - 1;//
            int num_pixels = params.OY0 * params.OX0;
            int ramp_up_time = IC0 - 1;
            int flush = OC0 - 1 ;
            int step_bound = ramp_up_time + num_pixels + flush;
            int step = 0;

            // -------------------------------
            // Create a loop for a "run" of the systolic array.
            // The number of steps in a run of the systolic array is equal to:
            // the ramp-up time + number of pixels + flush time
            // Your code starts here
            while (true) { 

            // Your code ends here 
            // You should now be in the body of the loop
            // -------------------------------

                // -------------------------------
                // If you are in the ramp up time, read in weights from the channel
                // and store it in the weights array
                // Your code starts here
                if (step <= ramp_up_time) {
                    weight_array.value[step] = weight.read();
                }

                // Your code ends here
                // -------------------------------
                
                
                PackedInt<INPUT_PRECISION, IC0> in_col;

                // -------------------------------
                // Read inputs from the channel and store in the variable in_col
                // Note: you don't read in any inputs during the flush time
                // Your code starts here
                    if (step <= num_pixels) {
                        in_col = input.read();
                    }
                // Your code ends here
                // -------------------------------

                // Debug example:        
                // printf("in_col: %s\n", in_col.to_string().c_str());


                /*
                 * FIFOs for inputs coming in to the systolic array
                 * assign values to in_col, and the skewed version will be in input_buf
                 */
                PackedInt<INPUT_PRECISION, IC0> input_buf;

                #define INPUT_FIFO_BODY(z,i,unused) \
                    IDTYPE BOOST_PP_CAT(input_fifo_output_, i); \
                    IDTYPE BOOST_PP_CAT(input_fifo_input_, i) = in_col.value[i]; \
                    BOOST_PP_CAT(input_fifo_, i).run( BOOST_PP_CAT(input_fifo_input_, i) , BOOST_PP_CAT(input_fifo_output_, i) ); \
                    input_buf.value[i] = BOOST_PP_CAT(input_fifo_output_, i);
                
                REPEAT(INPUT_FIFO_BODY)

                // -------------------------------
                // Assign values from input_buf into the registers for the first column of PEs
                // Your code starts here
                    ifmap_in.value[0] = input_buf;

                    // if (step < num_pixels + ramp_up_time) {
                    //     for (int i = 0; i < IC0; i++) {
                    //         input_reg[i][0] = input_buf.value[i];
                    //     }
                    // }

                // Your code ends here
                // -------------------------------

                PackedInt<OUTPUT_PRECISION, OC0> psum_buf;
                
                // -------------------------------
                // Set partial outputs for the array to psum_buf.
                // Depending on the loop index, the partial output will be 0 or a value from the accumulation buffer
                // Your code starts here
                if (step < num_pixels && loopIndices.ic1_idx != 0 && loopIndices.fx_idx == 0 && loopIndices.fy_idx == 0) {
                    psum_buf = accumulation_buffer.value[step];
                } else {
                    for (int i = 0; i < OC0; i++) {
                        psum_buf.value[i] = 0;
                    }
                }

                // Your code ends here
                // -------------------------------
                
                // Debug example:
                // printf("psum_buf: %s\n", psum_buf.to_string().c_str());

                /*
                 * FIFOs for partial outputs coming in to the systolic array
                 * assign values to psum_buf, and the skewed version will be in output_buf
                 */
                PackedInt<OUTPUT_PRECISION, OC0> output_buf;
                #define ACCUM_FIFO_BODY(z,i,unused) \
                    ODTYPE BOOST_PP_CAT(psum_fifo_output_, i); \
                    ODTYPE BOOST_PP_CAT(psum_fifo_input_, i) = psum_buf.value[i]; \
                    BOOST_PP_CAT(psum_fifo_, i).run( BOOST_PP_CAT(psum_fifo_input_, i) , BOOST_PP_CAT(psum_fifo_output_, i) ); \
                    output_buf.value[i] = BOOST_PP_CAT(psum_fifo_output_, i);
                
                REPEAT(ACCUM_FIFO_BODY)
        
                // -------------------------------
                // Assign values from output_buf into the partial sum registers for the first row of PEs
                // Your code starts here
                    ofmap_in.value[0] = output_buf;

                // Your code ends here
                // -------------------------------
            

                // -------------------------------
                // Run the 16x16 PE array
                // Make sure that the correct registers are given to the PE
                // Your code starts here
                for (int r = 0; r < OC0; r++) {
                    for (int c = 0; c < IC0; c++) {
                        pe_array[r][c].run(
                            ifmap_in.value[c].value[r], 
                            ofmap_in.value[r].value[c], 
                            weight_array.value[r].value[c], 
                            ifmap_out.value[c].value[r], 
                            ofmap_out.value[c].value[r]);
                    }
                }

                // Your code ends here
                // -------------------------------

                // Captures PE register state into log files
                #if HLS_DEBUG
                #ifndef __SYNTHESIS__
                log_matrix(input_file, input_reg, step, OC0);
                log_matrix(weight_file, weight_reg, step, OC0);
                log_matrix(psum_file, ofmap_out, step, OC0);
                #endif
                #endif
                

                /*
                 * FIFOs for partial outputs coming out of the systolic array
                 * The skewed version will be in the variable output_row
                 */
                PackedInt<OUTPUT_PRECISION, OC0> output_row;

                #define FIFO_WRITE_BODY_NEW(z,i,unused)\
                    ODTYPE BOOST_PP_CAT(accum_fifo_output_, i); \
                    BOOST_PP_CAT(accum_fifo_, i).run( ofmap_out.value[IC0-1].value[i] , BOOST_PP_CAT(accum_fifo_output_, i) );\
                    output_row.value[i] = BOOST_PP_CAT(accum_fifo_output_,i); \
                
                REPEAT(FIFO_WRITE_BODY_NEW)

                // -------------------------------
                // After a certain number of cycles, you will have valid output from the systolic array
                // Depending on the loop indices, this valid output will either be written into the accumulation buffer or written out
                // Your code starts here
                if (step >= ramp_up_time + flush) {
                    if ((loopIndices.ic1_idx == params.IC1 - 1) && (loopIndices.fx_idx == params.FX - 1) && (loopIndices.fy_idx == params.FY - 1)) {
                        output.write(output_row);
                    } else {
                        accumulation_buffer.value[step - (IC0 + OC0)] = output_row;
                    }
                }

                // Your code ends here
                // -------------------------------
                
                // -------------------------------
                // Cycle the input/psum registers
                // That is, the outputs that a PE wrote to should now become the input for the next PE
                // Your code starts here
                for (int i = 0; i < IC0; i++) {
                    ifmap_in.value[i+1] = ifmap_out.value[i];
                }
                for (int i = 0; i < OC0; i++) {
                    ofmap_in.value[i+1] = ofmap_out.value[i];
                }
                // Your code ends here
                // -------------------------------
                if (step == step_bound-1) break;
                step++;
                
            }
        }
    
        // Debug example:
        // printf("outputs written: %d\n", output.size());
    }

private:
    
    // -------------------------------
    // Create the following:
    //  - PE array
    //  - accumulation buffer
    //  - weight registers
    //  - input registers (two sets, one at the input of the PE and one at the output) 
    //  - psum registers (two sets, one at the input of the PE and one at the output) 
    // Your code starts here

    ProcessingElement<IDTYPE, WDTYPE, ODTYPE> pe_array[OC0][IC0];

    PackedInt2D<WEIGHT_PRECISION, OC0, IC0> weight_array;
    PackedInt2D<INPUT_PRECISION, IC0, OC0> ifmap_in;
    PackedInt2D<OUTPUT_PRECISION, OC0, ACCUMULATION_BUFFER_SIZE> accumulation_buffer;
    PackedInt2D<OUTPUT_PRECISION, OC0, IC0> ofmap_in;
    PackedInt2D<INPUT_PRECISION, IC0, OC0> ifmap_out;
    PackedInt2D<OUTPUT_PRECISION, OC0, IC0> ofmap_out;
    // Your code ends here
    // -------------------------------
    

#define INPUT_FIFOS_INIT(z, i, unused) \
    Fifo<IDTYPE, i + 1> BOOST_PP_CAT(input_fifo_, i);

    REPEAT(INPUT_FIFOS_INIT)

#define ACCUM_FIFOS_INIT(z, i, unused) \
    Fifo<ODTYPE, i + 1> BOOST_PP_CAT(psum_fifo_, i);

    REPEAT(ACCUM_FIFOS_INIT)
    

#define OUTPUT_FIFOS_INIT(z, i, unused) \
    Fifo<ODTYPE, OC0 - i> BOOST_PP_CAT(accum_fifo_, i);
    
    REPEAT(OUTPUT_FIFOS_INIT)
};

#endif
