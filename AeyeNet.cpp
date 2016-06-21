#include "AeyeNet.hpp"


int memType::PutMem(void* src){
	memcpy((void* )(this->mem_start),src,this->memlen);
	std::cout << "set input 3" << std::endl;
	return 0;
}

int memType::PutMem(std::string fname){
	std::cout << "set input 2" << std::endl;
	std::cout << this->memlen << std::endl;
	std::cout << this->mem_start << std::endl;
	FILE* infile = fopen(fname.c_str(),"rb");
	if( !infile ){ printf("openfile %s error \n",fname.c_str()); exit(-1);}
	fread((void*)(this->mem_start),1,this->memlen,infile);
	fclose(infile);
	return 0;
}

int memType::showMem(void* dest){
	memcpy(dest,(void* )mem_start,this->memlen);
	return 0;
}

int memType::showMem(std::string fname){
	std::ofstream ofs;
	ofs.open(fname.c_str(),std::ios::trunc);
	for(int i=0;i < this->memlen;i++){
		ofs << (int)(char)mem_start[i] <<std::endl;
	}
	ofs.close();
	return 0;
}



void Aeyelayer::init(uint8_t * startweight,uint8_t * startbias,uint8_t * startinput,uint8_t * startoutput,uint8_t * startinstru, \
			int lenweight,int lenbias,int leninput,int lenoutput,int leninst){
	uint8_t * starttype[5];
	int sizetype[5];
	starttype[0] = startweight;sizetype[0] = lenweight;
	starttype[1] = startbias;sizetype[1] = lenbias;
	starttype[2] = startinput;sizetype[2] = leninput;
	starttype[3] = startoutput;sizetype[3] = lenoutput;
	starttype[4] = startinstru;sizetype[4] = leninst;
	init(starttype,sizetype);
}




void AeyeNet::SetLayerNum(int n){
	numLayer = n;
	startweights = (uint8_t**) malloc(sizeof(uint8_t*)*n);
	startinput = (uint8_t**) malloc(sizeof(uint8_t*)*n);
	startbias = (uint8_t**) malloc(sizeof(uint8_t*)*n);
	startoutput = (uint8_t**) malloc(sizeof(uint8_t*)*n);	
	startinstruction = (uint8_t**) malloc(sizeof(uint8_t*)*n);

	lenweights = (int*) malloc(sizeof(int)*n);
	leninput = (int*) malloc(sizeof(int)*n);
	lenbias = (int*) malloc(sizeof(int)*n);
	lenoutput = (int*) malloc(sizeof(int)*n);
	leninstruction = (int*) malloc(sizeof(int)*n);


	for(int i=0;i<n;i++){
		AeyeLayer tmpLayer = AeyeLayer();
		this->LayerList.push_back(tmpLayer);
	}
};


void AeyeNet::SetInput(uint8_t** s_input,int* len_intput){
	for(int i=0;i<numLayer;i++){
		startinput[i]=s_input[i];
		leninput[i] = len_intput[i];
	}
};
void AeyeNet::SetWeights(uint8_t** s_weights,int* len_weights){
	for(int i=0;i<numLayer;i++){
		startweights[i]=s_weights[i];
		lenweights[i] = len_weights[i];
	}
};
void AeyeNet::SetBias(uint8_t** s_bias,int* len_bias){
	for(int i=0;i<numLayer;i++){
		startbias[i]=s_bias[i];
		lenbias[i] = len_bias[i];
	}
};
void AeyeNet::SetOutput(uint8_t** s_output,int* len_output){
	for(int i=0;i<numLayer;i++){
		startoutput[i]=s_output[i];
		lenoutput[i] = len_output[i];
	}
};
void AeyeNet::SetInstuction(uint8_t** s_instuctions,int* len_inst){
	for(int i=0;i<numLayer;i++){
		startinstruction[i]=s_instuctions[i];
		leninstruction[i] = len_inst[i];
	}
};

void Aeyenet::InitLayers(){
	for (int i=0;i<numLayer;i++){
		this->LayerList[i].init(startweights[i],startbias[i],startinput[i],startoutput[i],startinstruction[i],\
				lenweights[i],lenbias[i],leninput[i],lenoutput[i],leninstruction[i]);
	}

}


int AeyeNet::FPGA_reg_map(){
	int fd = open("/dev/mem",O_RDWR|O_SYNC);
	if(fd<0) {
		printf("error open()");
		exit(0);
	}

	off_t off = XLED_BASEADDR;
	size_t len = XLED_HIGHADDR - XLED_BASEADDR + 1;
	void *vptr = mmap(NULL,len,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_POPULATE,fd,off);
	if(vptr == MAP_FAILED){
		printf("error mmap()");
		return -2;
	}
	p_fpga_reg_start = (uint32_t *)off;
	v_fpga_reg_start = (uint32_t *)vptr;
	sg_FPGA_regs = (S_FPGA_LED *) v_fpga_reg_start;
	fpga_reg_len = len;

	off = XPRO_BASEADDR;
	len = XPRO_HIGHADDR - XPRO_BASEADDR + 1;
	vptr = mmap(NULL,len,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_POPULATE,fd,off);
	if(vptr == MAP_FAILED){
		printf("error mmap()");
		return -2;
	}
	p_fpga_profile_start = (uint32_t *)off;
	v_fpga_profile_start = (uint32_t *)vptr;
	profile_FPGA_regs = (S_FPGA_PROFILE *) v_fpga_profile_start;
	fpga_profile_len = len;

	off = XMEM_BASEADDR;
	len = XMEM_HIGHADDR - XMEM_BASEADDR + 1;
	vptr = mmap(NULL,len,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_POPULATE,fd,off);
	if(vptr == MAP_FAILED){
		printf("error mem mmap()");
		return -2;
	}
	p_mem_start = (uint8_t *)off;
	v_mem_start = (uint8_t *)vptr;
	mem_start = v_mem_start;
	inst_start = mem_start + (INST_BASEADDR - XMEM_BASEADDR);
	mem_len = len;
	return 0;
}

int AeyeNet::FPGA_reg_map(off_t memstart,off_t inststart){
	FPGA_reg_map();
	mem_start = (memstart-XMEM_BASEADDR)+mem_start;
	inst_start = (inststart-INST_BASEADDR)+inst_start;
}


void AeyeNet::run_inst(int inst_len,int single_64_ins,uint32_t inst_start) {
	long long int t1,t2;
	int ins_full_trance = inst_len / single_64_ins;
	int ins_last_trace = inst_len % single_64_ins;
	//waitIdle();
	//std::cout << ins_full_trance << "  "<< ins_last_trace << std::endl;
	for (int i = 0;i<ins_full_trance;i++){
		sg_FPGA_regs->reg0 = uint32_t(inst_start + i*8*single_64_ins);
		//std::cout<<std::hex << uint32_t(inst_start + i*8*single_64_ins) << std::endl;
		sg_FPGA_regs->reg1 = single_64_ins;
		if(i == 0){
			while( ((sg_FPGA_regs->reg2) & 0x007) ) {
				//printf(" reg3 is temp %x\n", (sg_FPGA_regs->reg3));
			}
		}
		else {
			while( ((sg_FPGA_regs->reg2) & 0x007) ){
				//printf(" reg3 is temp %x\n", (sg_FPGA_regs->reg3));
			}
		}
	}
	if(ins_last_trace > 0) {
		sg_FPGA_regs->reg0 = uint32_t(inst_start + ins_full_trance*8*single_64_ins);
		sg_FPGA_regs->reg1 = ins_last_trace;
		while( ((sg_FPGA_regs->reg2) & 0x007) ){
			//printf(" reg3 is temp %x", (sg_FPGA_regs->reg3) );
		}
	}
	waitIdle();
}



void AeyeNet::waitIdle() {
	int state = 0;
	int tempstate;
	while( state < 3 ) {
		if((tempstate=sg_FPGA_regs->reg3) == 0x44000000) {
			state ++;	
			usleep(10);
		}
		else {
			//std::cout << "waitIdle "<< std::hex << tempstate << std::endl;	
			usleep(10);
			state = 0;
		}
	}
}

void AeyeNet::reset_aeye(){
	(profile_FPGA_regs->reg1) = 0x0;
	usleep(10);
	(profile_FPGA_regs->reg1) = 0xf;
	
}
int AeyeNet::get_profile(int max_len,int* laod_data,int* save_data,int* calc_data){
	long long unsigned int temp_addr;
	long long unsigned int data_64;
	bool valid;

	for (int i=0; i<max_len; i++){
		valid = false;
		(profile_FPGA_regs->reg0) = i;
		usleep(10);

		temp_addr = profile_FPGA_regs->reg2;
		data_64 = temp_addr & 0xffffffff;
		temp_addr = profile_FPGA_regs->reg3;
		data_64 |= ((temp_addr & 0xffffffff)<<32);
		if(data_64 & 0x8000000000000000){
			laod_data[i * 3] = (data_64 & 0xfffffff);
			laod_data[i * 3 + 1] = ((data_64 >> 28) & 0xfffffff);
			laod_data[i * 3 + 2] = ((data_64 >> 56) & 0x3);
			valid = true;
		}

		temp_addr = profile_FPGA_regs->reg4;
		data_64 = temp_addr & 0xffffffff;
		temp_addr = profile_FPGA_regs->reg5;
		data_64 |= ((temp_addr & 0xffffffff)<<32);
		if(data_64 & 0x8000000000000000){
			save_data[i * 3] = (data_64 & 0xfffffff);
			save_data[i * 3 + 1] = ((data_64 >> 28) & 0xfffffff);
			save_data[i * 3 + 2] = ((data_64 >> 56) & 0x3);
			valid = true;
		}

		temp_addr = profile_FPGA_regs->reg6;
		data_64 = temp_addr & 0xffffffff;
		temp_addr = profile_FPGA_regs->reg7;
		data_64 |= ((temp_addr & 0xffffffff)<<32);
		if(data_64 & 0x8000000000000000){
			calc_data[i * 3] = (data_64 & 0xfffffff);
			calc_data[i * 3 + 1] = ((data_64 >> 28) & 0xfffffff);
			calc_data[i * 3 + 2] = ((data_64 >> 56) & 0x3);
			valid = true;
		}		
		if(!valid){
			return i;
		}
	}
	return max_len;
}
