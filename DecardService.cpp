// DecardService.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。

#include <iostream>
#include "httplib.h"
#include <windows.h>
#include "DecardSDK/dcic32.h"
#include <stdio.h>

using namespace std;


class DecardReader
{
public:
	HANDLE icdev;
	int init_device(void);
	int close_device(void);
	long long of_read_rfid_card(void);
	string of_read_insur_card_no(void);
	void beep(void);
};
void DecardReader::beep(void) {
	int beep_status = IC_DevBeep(icdev, 15);
}


int DecardReader::init_device(void) {
	icdev = IC_InitCommAdvanced(100); // usb
	if (int(icdev) <= 0)
	{
		std::cout << "Failed to connect Card Reader" << std::endl;
		return 0;
	}
	return 1;
}

int DecardReader::close_device(void) {
	__int16 close_status = IC_ExitComm(icdev);
	if (close_status < 0) {
		std::cout << "Failed to close Device" << std::endl;
		return 0;
	}
	return 1;
}

long long DecardReader::of_read_rfid_card(void) {
	int st = -1;
	DWORD cardsnr = -1;
	short w_tag_type = -1;
	st = IC_Request(icdev, 0x0, &w_tag_type);

	if (st != 0)
	{
		std::cout << "request Card Error!";
		return -1;
	}
	st = IC_Anticoll(icdev, 0, &cardsnr);
	if (st != 0)
	{
		std::cout << "anticoll Error!";
		return -2;
	}

	unsigned char sss[5] = "\0";
	st = IC_Select(icdev, cardsnr, sss);
	if (st != 0)
	{
		std::cout << "dc_select Error!";
		return -3;
	}

	int ai_bsecnr = -1;
	unsigned char bNkey[20] = "FFFFFFFFFFFF\0";
	st = IC_Load_Keyhex(icdev, 0, ai_bsecnr, bNkey);
	if (st < 0) {
		std::cout << "IC_Load_Key Error!";
		return -4;
	}

	st = IC_Authentication(icdev, 0, 0);//验证第0扇区的A套密码
	if (st != 0)
	{
		std::cout << "IC_Authentication Error!";
		return -5;
	}

	BYTE readData[50] = "\0";
	st = IC_ReadMifare_Hex(icdev, 0, readData);//读取M1卡第一块的数据
	cout << "sector 0 data"<<readData << endl;
	if (st != 0)
	{
		std::cout << "IC_ReadMifare Error!";
		return -6;
	}
	BYTE transData[50] = "\0";
	transData[0] = readData[6];
	transData[1] = readData[7];
	transData[2] = readData[4];
	transData[3] = readData[5];
	transData[4] = readData[2];
	transData[5] = readData[3];
	transData[6] = readData[0];
	transData[7] = readData[1];
	transData[8] = '\0';
	cout << "transData :"<<transData << endl;
	unsigned long card_no;
	sscanf_s((char*)transData, "%x", &card_no);
	beep();
	return card_no;
}


string DecardReader::of_read_insur_card_no() {
	int st = -1;
	st = IC_Status(icdev);
	std::cout << "ic stauts: " << st << std::endl;
	if (st  == 1) {
		return "-101";
	}

	if (st< 0) {
		return "-102";
	}
	

	unsigned char slen;
	unsigned char rlen;
	unsigned char databuffer[2048];

	st = IC_InitType(icdev, 0x0c);
	if (st < 0) {
		return "-103";
	}
	std::cout << "Init status " << st << std::endl;
	st = IC_CpuReset_Hex(icdev, &rlen, databuffer);

	std::cout << "reset code:" << st << std::endl;
	cout << "rlen " << (int)rlen << endl;
	std::cout << databuffer << std::endl;
	if (st < 0) {
		return "-104";
	}


	string cmdstr = "00A404000F7378312E73682EC9E7BBE1B1A3D5CF";

	slen = cmdstr.length()/2;
	rlen = -1;
	cout << "slen: " << (int)slen << endl;
	cout << "sendbuffer "<<(unsigned char*)cmdstr.c_str() << endl;
	st = IC_CpuApdu_Hex(icdev, slen, (unsigned char *)cmdstr.c_str(), &rlen, databuffer);

	cout << "Command Status: " <<st<<endl;
	cout << "rlen " << (int)rlen << endl;
	cout << "data: " << databuffer << endl;
	if (st < 0) {
		return "-105";
	}

	cmdstr = "00A4020002EF05";
	slen = cmdstr.length() / 2;
	rlen = -1;
	cout << "slen: " << (int)slen << endl;
	cout << "sendbuffer " << (unsigned char*)cmdstr.c_str() << endl;
	st = IC_CpuApdu_Hex(icdev, slen, (unsigned char*)cmdstr.c_str(), &rlen, databuffer);
	cout << "Command Status: " << st << endl;
	cout << "rlen " << (int)rlen << endl;
	cout << "data: " << databuffer << endl;
	if (st < 0) {
		return "-106";
	}

	cmdstr = "00B207000B";
	slen = cmdstr.length() / 2;
	rlen = -1;
	cout << "slen: " << (int)slen << endl;
	cout << "sendbuffer " << (unsigned char*)cmdstr.c_str() << endl;
	st = IC_CpuApdu_Hex(icdev, slen, (unsigned char*)cmdstr.c_str(), &rlen, databuffer);
	cout << "Command Status: " << st << endl;
	cout << "rlen " << (int)rlen << endl;
	cout << "data: " << databuffer << endl;
	if (st < 0) {
		return "-107";
	}
	char ascbuffer[1024];
	databuffer[rlen*2 -4 +1] = '\0';
	hex2asc(databuffer+4, (unsigned char *)ascbuffer, 9);
	ascbuffer[9] = '\0';
	beep();
	return string(ascbuffer);
}

void test() {
	DecardReader reader;
	reader.init_device();
	cout<<reader.of_read_rfid_card()<<endl<<endl;
	cout<<reader.of_read_insur_card_no();
	reader.close_device();
}


int main()
{

	httplib::Server svr;


	svr.Get("/read_card", [](const httplib::Request&, httplib::Response& res) {
		clock_t start, finish;
		start = clock();
		DecardReader reader;
		reader.init_device();
		long long rfid_card_no = reader.of_read_rfid_card();
		string insur_card_no = reader.of_read_insur_card_no();
		finish = clock();

		int code = 200;
		if (rfid_card_no < 0 && insur_card_no.length() == 3) {
			code = 503;
		}
		char data[1024];
		sprintf_s(
			data,
			"{\"code\": %d, \"rfid_card_no\": \"%010lld\", \"insur_card_no\":\"%s\", \"time\": \"%.0fms\"}",
			code,
			rfid_card_no,
			insur_card_no.c_str(),
			float(finish) - float(start)
			);
		res.set_content(data, "application/json");
		reader.close_device();
	});

	svr.listen("0.0.0.0", 8080);
	return 0;
}



// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件