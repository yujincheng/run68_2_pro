#include <string>
#include <vector>

#include <malloc.h>
#include <sstream>
#include <iomanip>
#include "align68.hpp"


using namespace std;


int main(){
	align68 AL68;
	cout << "main:" <<0 << endl;
    	AL68.init();
	cout << "main:" <<1 << endl;
	//for(int i = 1;i <10;i++){
	AL68.Anet.reset_aeye();
	AL68.run_conv();
	//}
	cout << "main" << 2 << endl;
	AL68.Anet.LayerList[8].output.showMem(std::string("out_8.txt"));


//---------------------------------------------------------------------	
	int* laod_data = new int[1024*3];
	int* save_data = new int[1024*3];
	int* calc_data = new int[1024*3];
	for(int i = 0;i < 1024*3;i++){
		laod_data[i] = 0;
		save_data[i] = 0;
		calc_data[i] = 0;
	}
	AL68.Anet.get_profile(1000,laod_data,save_data,calc_data);
	std::ofstream ofs;
	ofs.open("laod_data.txt",std::ios::trunc);
	for(int i=0;i < 1024*3;i++){
		ofs << (int)laod_data[i] <<std::endl;
	}
	ofs.close();
	ofs.open("save_data.txt",std::ios::trunc);
	for(int i=0;i < 1024*3;i++){
		ofs << (int)save_data[i] <<std::endl;
	}
	ofs.close();
	ofs.open("calc_data.txt",std::ios::trunc);
	for(int i=0;i < 1024*3;i++){
		ofs << (int)calc_data[i] <<std::endl;
	}
	ofs.close();
	
//--------------------------------------------------------------------
	return 0;

}
