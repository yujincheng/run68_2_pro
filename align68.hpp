#include "AeyeNet.hpp"
#include <malloc.h>
#include <sstream>
#include <iomanip>
using namespace std;
typedef struct Align68{
	const int mem_start_p = 0x10000000;
	const int inst_start_p = 0x15000000;
    int mem_start;
    int inst_start;
    AeyeNet Anet;
    float* result;
    float* fc_weights;
    float* fc_bias;
    char* beforepull;
  	float* afterpull;
    void init();
    void run_conv();
    void run_fc(float*);
  int* bias_addr;
  int* bias_size;
  int* weights_addr;
  int* weights_size;
  int* input_addr;
  int* input_size;
  int* output_addr;
  int* output_size;
  int* inst_addr ;
  int* inst_size ;
}align68;

void Align68::init(){
  result = new float[136];
  fc_weights = new float[3*3*160*136];
  fc_bias = new float[136];
  beforepull = new char[8*6*160];
	afterpull = new float[3*3*160];
//  FILE* infile = fopen("param/fc/weights/weights.bin","rb");
//	if( !infile ){ printf("openfile fc weights error \n"); exit(-1);}
//	fread(fc_weights,sizeof(float),3*3*160*136,infile);
//	fclose(infile);
//	infile = fopen("param/fc/bias/bias.bin","rb");
//	if( !infile ){ printf("openfile fc bias error \n"); exit(-1);}
//	fread(fc_bias,sizeof(float),136,infile);
//	fclose(infile);



	Anet.FPGA_reg_map(mem_start_p,inst_start_p);
mem_start = Anet.mem_start;
inst_start = Anet.inst_start;

  int alllayer = 0;
  ifstream ifs;
  ifs.open("./all_compiler.txt");
  ifs >> alllayer;
  cout << "all layer count " <<  alllayer << endl;

  bias_addr = new int[alllayer];
  bias_size = new int[alllayer];
  weights_addr = new int[alllayer];
  weights_size = new int[alllayer];
  input_addr = new int[alllayer];
  input_size = new int[alllayer];
  output_addr = new int[alllayer];
  output_size = new int[alllayer];
  inst_addr = new int[alllayer];
  inst_size = new int[alllayer];
  for (int i=0;i<alllayer;i++){
      ifs >> bias_addr[i];
      bias_addr[i] += mem_start;
  }
  for (int i=0;i<alllayer;i++){
      ifs >> bias_size[i];
  }
  for (int i=0;i<alllayer;i++){
      ifs >> weights_addr[i];
      weights_addr[i] += mem_start;
  }
  for (int i=0;i<alllayer;i++){
      ifs >> weights_size[i];
  }
  for (int i=0;i<alllayer;i++){
      ifs >> input_addr[i];
      input_addr[i] += mem_start;
  }
  for (int i=0;i<alllayer;i++){
      ifs >> input_size[i];
  }
  for (int i=0;i<alllayer;i++){
      ifs >> output_addr[i];
      output_addr[i] += mem_start;
  }
  for (int i=0;i<alllayer;i++){
      ifs >> output_size[i];
  }
  for (int i=0;i<alllayer;i++){
      ifs >> inst_addr[i];
      inst_addr[i] += inst_start;
  }
  for (int i=0;i<alllayer;i++){
      ifs >> inst_size[i];
  }
  ifs.close();

	Anet.SetLayerNum(alllayer);


	uint8_t** bias_list = new uint8_t*[alllayer];
	uint8_t** weight_list = new uint8_t*[alllayer];
	uint8_t** input_list = new uint8_t*[alllayer];
	uint8_t** output_list = new uint8_t*[alllayer];
	uint8_t** inst_list = new uint8_t*[alllayer];
	for(int i = 0;i<alllayer;i++){
		bias_list[i] = (uint8_t*)(bias_addr[i]);
		weight_list[i] = (uint8_t*)(weights_addr[i]);
		input_list[i] = (uint8_t*)(input_addr[i]);
		output_list[i] = (uint8_t*)(output_addr[i]);
		inst_list[i] = (uint8_t*)(inst_addr[i]);
	}
  Anet.SetBias(bias_list,bias_size);
	Anet.SetWeights(weight_list,weights_size);
	Anet.SetInput(input_list,input_size);
	Anet.SetOutput(output_list,output_size);
	Anet.SetInstuction(inst_list,inst_size);
	Anet.InitLayers();

	for(int i = 0 ; i < alllayer ; i++){
		std::stringstream ss;
		ss << i+1;
		std::string count;
		ss >> count;
		std::cout <<"layer" << i << std::endl;
		if(i==0){
		//std::cout << "running" << std::endl;
		Anet.LayerList[i].input.PutMem(std::string(std::string("./input/outinput.bin")));
    }
		//std::cout <<"finish input" << i << std::endl;
		Anet.LayerList[i].weights.PutMem(std::string("./param/conv")+count+std::string("/weights/weight.bin"));
		//std::cout <<"finish weights" << i << std::endl;
		Anet.LayerList[i].bias.PutMem(std::string("./param/conv")+count+std::string("/bias/bias.bin"));
		//std::cout <<"finish bais" << i << std::endl;
		Anet.LayerList[i].instruction.PutMem(std::string("./param/conv")+count+std::string("/command.dat"));
		//std::cout <<"finish inst" << i << std::endl;
	}
}

void Align68::run_conv(){
  int prelen = 0;
  long long int t1,t2;
  int totallen = 0;
	for (int i=0;i<9;i++){
		//Anet.run_inst(inst_size[i]/8,64,inst_start_p+prelen);prelen += inst_size[i];
		totallen += inst_size[i];
	}
	Anet.run_inst(totallen/8,64,inst_start_p);	
}

void Align68::run_fc(float* outresult){
	for (int i=0;i<3*3*160;i++){
		afterpull[i] = 0;
	}

	Anet.LayerList[8].output.showMem((void*)(beforepull));
	int count_pos = 0;
	for (int  index = 0; index < 8*6*160 ; index++){
		int channel_count = int(index/48);
		int inpic = index - channel_count*48;
		int y_pos = int(inpic/8);
		int x_pos = inpic - y_pos*8;
		if(x_pos > 5){continue;};
		int x_out_pos = int(x_pos/2);
		int y_out_pos = int(y_pos/2);
		afterpull[x_out_pos + y_out_pos*3 + channel_count*9] += (float)beforepull[index];
		count_pos ++;
	}
	for (int i=0;i<3*3*160;i++){
		afterpull[i] /= 512;
	}
	for (int i=0;i<136;i++){
		outresult[i] = 0;
	}
	for (int i=0; i<3*3*160; i++) {
		for (int j=0; j<136; j++) {
			outresult[j] += fc_weights[i*136+j] * afterpull[i];
		}
	}
	for (int j=0; j<136; j++) {
		outresult[j] += fc_bias[j];
	}
}
