#ifndef _AEYENET_HPP_
#define _AEYENET_HPP_

#include <iostream>
#include <cstdlib>
#include <string>
#include <string.h>
#include <fstream>
#include <stdint.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>  // for close()

typedef struct memType{
	volatile uint8_t * mem_start;
	int memlen;
	int PutMem(void*);
	int PutMem(std::string);
	int showMem(void*);
	int showMem(std::string);
	memType(){mem_start=0;memlen=0;};
	memType(uint8_t * mem_start_in,int memlen_in){this->mem_start = mem_start_in;this->memlen = memlen_in;};
	memType(uint8_t * mem_start_in,off_t offset,int memlen_in){this->mem_start = mem_start_in+offset;this->memlen = memlen_in;};
	void init(uint8_t * mem_start_in,off_t offset,int memlen_in){this->mem_start = mem_start_in+offset;this->memlen = memlen_in;};
	void init(uint8_t * mem_start_in,int memlen_in){this->mem_start = mem_start_in;this->memlen = memlen_in;};

}memtype;



typedef struct Aeyelayer
{
	memType weights;
	memType bias;
	memType input;
	memType output;
	memType instruction;
	
	void init(uint8_t * starttype[],int sizetype[]){weights.init(starttype[0],sizetype[0]);
													bias.init(starttype[1],sizetype[1]);
													input.init(starttype[2],sizetype[2]);
													output.init(starttype[3],sizetype[3]);
													instruction.init(starttype[4],sizetype[4]);};

	void init(uint8_t * startweight,uint8_t * startbias,uint8_t * startinput,uint8_t * startoutput,uint8_t * startinstru, \
				int lenweight,int lenbias,int leninput,int lenoutput,int leninst);
}AeyeLayer;


typedef struct FPGA_led{
	volatile uint32_t reg0; //0x00
	volatile uint32_t reg1; //0x04
	volatile uint32_t reg2; //0x08
	volatile uint32_t reg3; //0x0C
}S_FPGA_LED;

typedef struct FPGA_profile{
	volatile uint32_t reg0; //0x00
	volatile uint32_t reg1; //0x04
	volatile uint32_t reg2; //0x08
	volatile uint32_t reg3; //0x0C
	volatile uint32_t reg4; //0x10
	volatile uint32_t reg5; //0x14
	volatile uint32_t reg6; //0x18
	volatile uint32_t reg7; //0x1C
}S_FPGA_PROFILE;

typedef struct AeyeNet
{
	int numLayer;
	std::vector<Aeyelayer> LayerList;
	
	uint32_t *p_fpga_reg_start;
	uint32_t *v_fpga_reg_start;
	size_t fpga_reg_len;


	uint32_t *p_fpga_profile_start;
	uint32_t *v_fpga_profile_start;
	size_t fpga_profile_len;

	uint8_t * p_mem_start;
	uint8_t * v_mem_start;
	size_t mem_len;

	
	volatile uint8_t * mem_start;
	volatile uint8_t * inst_start;
	volatile S_FPGA_LED *sg_FPGA_regs;
	volatile S_FPGA_PROFILE *profile_FPGA_regs;

	
	static const off_t XLED_BASEADDR = 0x43C00000;
	static const off_t XLED_HIGHADDR = 0x43C0FFFF;
	static const off_t XPRO_BASEADDR = 0x43C10000;
	static const off_t XPRO_HIGHADDR = 0x43C1FFFF;
	static const off_t XMEM_BASEADDR = 0x10000000;
	static const off_t XMEM_HIGHADDR = 0x1FFFFFFF;
	static const off_t INST_BASEADDR = 0x15000000;


	/*************************************START_ADDR************************************************************/

	uint8_t** startweights;
	uint8_t** startinput;
	uint8_t** startbias;
	uint8_t** startoutput;
	uint8_t** startinstruction;


	/************************************LENGTH************************************************************/
	
	int* lenweights;
	int* leninput;
	int* lenbias;
	int* lenoutput;
	int* leninstruction;

	void SetLayerNum(int n);
	void SetInput(uint8_t** s_input,int* len_intput);
	void SetWeights(uint8_t** s_weights,int* len_weights);
	void SetBias(uint8_t** s_bias,int* len_bias);
	void SetOutput(uint8_t** s_output,int* len_output);
	void SetInstuction(uint8_t** s_instuctions,int* len_inst);

	void InitLayers();
	void reset_aeye();

	void waitIdle();

	int FPGA_reg_map();
	int FPGA_reg_map(off_t,off_t);

	void run_inst(int inst_len,int single_64_ins,uint32_t inst_start);

	int get_profile(int max_len,int* laod_data,int* save_data,int* calc_data);
}Aeyenet;


#endif
