#include <string.h>
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestRegistry.h"
#include "CppUTestExt/MockSupport.h"

#include "XModem.h"
int main(int ac, const char** av)
{
	return CommandLineTestRunner::RunAllTests(ac, av);
}

int recvChar(int msDelay)
{
	if (mock().hasData("recvChar"))
		return mock().actualCall("recvChar").returnValue().
							getIntValue();
	else
		return -1;

}
void sendChar(char sym)
{
	mock().actualCall("sendChar").withParameter("sym", sym);
}
bool dataHandler(unsigned long no, char* data, int size)
{
	if (mock().hasData("dataHandler"))
	{	if(mock().getData("dataHandler").getIntValue() == 1)
			return (mock().actualCall("dataHandler").
				withParameter("no", (int)no).
				withParameter("data", data[0]).
				withParameter("size", size).
				returnValue().getIntValue() == 1)?
				true:false;						
	
		if(mock().getData("dataHandler").getIntValue() == 2)
			return (mock().actualCall("dataHandler").
				withParameter("no", (int)no).
				withParameter("size", size).
				returnValue().getIntValue() == 1)?
				true:false;						
	
		if(mock().getData("dataHandler").getIntValue() == 3)
		{
			memset((void*)data, mock().getData("dataHandler_data").getIntValue(), 128);	
			return (mock().actualCall("dataHandler").
				withParameter("no", (int)no).
				withParameter("size", size).
				returnValue().getIntValue() == 1)?
				true:false;						
		}
							
	}
	else
		return true;
}

TEST_GROUP(xmodemrcv)
{

	void setup()
	{
	}

	void teardown()
	{
		mock().checkExpectations();
	mock().clear();	
	}
	
};

TEST(xmodemrcv, expect_C_Nack)
{
	mock().setData("recvChar", 1);
	mock().expectNCalls(16, "sendChar").withParameter("sym", 'C');
	mock().expectNCalls(16, "recvChar").andReturnValue(-1);
	mock().expectNCalls(16, "sendChar").withParameter("sym", XModem::NACK);
	mock().expectNCalls(16, "recvChar").andReturnValue(-1);
	XModem modem(recvChar, sendChar);
	
	modem.receive();

	
	

}
TEST(xmodemrcv, invalid_cmd)
{
	mock().setData("recvChar", 1);
	
	mock().expectNCalls(1, "sendChar").withParameter("sym", 'C');
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH+1);
	//expect 3 CAN 
	mock().expectNCalls(3, "sendChar").withParameter("sym", XModem::CAN);
	
	XModem modem(recvChar, sendChar);
	

	CHECK(false == modem.receive());
}
TEST(xmodemrcv, transfer_1_block_crc)
{
	mock().setData("recvChar", 1);
	
	mock().expectNCalls(1, "sendChar").withParameter("sym", 'C');
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//some crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0x1C);
	mock().expectNCalls(1, "recvChar").andReturnValue(0xCE);

	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);
	//end of transfer
	mock().expectOneCall("recvChar").andReturnValue(XModem::EOT);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	XModem modem(recvChar, sendChar);
	
	CHECK(true == modem.receive());

}

TEST(xmodemrcv, transfer_2_block_crc)
{
	mock().setData("recvChar", 1);
	
	mock().expectNCalls(1, "sendChar").withParameter("sym", 'C');
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0x1C);
	mock().expectNCalls(1, "recvChar").andReturnValue(0xCE);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);
	
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(2);
	mock().expectOneCall("recvChar").andReturnValue(253);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('B');
	//some crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0xDF);
	mock().expectNCalls(1, "recvChar").andReturnValue(0x8F);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);
	//end of transfer
	mock().expectOneCall("recvChar").andReturnValue(XModem::EOT);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	XModem modem(recvChar, sendChar);
	
	CHECK(true == modem.receive());

}
TEST(xmodemrcv, transfer_1_block_chksum)
{
	mock().setData("recvChar", 1);
	
	mock().expectNCalls(16, "sendChar").withParameter("sym", 'C');
	mock().expectNCalls(16, "recvChar").andReturnValue(-1);
	
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::NACK);
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//chksum
	mock().expectNCalls(1, "recvChar").andReturnValue(128);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	//end of transfer
	mock().expectOneCall("recvChar").andReturnValue(XModem::EOT);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	XModem modem(recvChar, sendChar);
	
	CHECK(true == modem.receive());

}
TEST(xmodemrcv, transfer_1_block_retransmit_no_data)
{
	mock().setData("recvChar", 1);
	
	mock().expectNCalls(1, "sendChar").withParameter("sym", 'C');
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	mock().expectNCalls(100, "recvChar").andReturnValue('A');
	mock().expectNCalls(1, "recvChar").andReturnValue(-1);

	//invalid read - expect NACK
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::NACK);

	//retry transfer!

	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//some crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0x1C);
	mock().expectNCalls(1, "recvChar").andReturnValue(0xCE);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);
	//end of transfer
	mock().expectOneCall("recvChar").andReturnValue(XModem::EOT);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	XModem modem(recvChar, sendChar);
	
	CHECK(true == modem.receive());

}
TEST(xmodemrcv, transfer_1_block_retransmit_blockno)
{
	mock().setData("recvChar", 1);
	
	mock().expectNCalls(1, "sendChar").withParameter("sym", 'C');
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(2);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//invalid number - expect NACK
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::NACK);

	//retry transfer!

	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//some crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0x1C);
	mock().expectNCalls(1, "recvChar").andReturnValue(0xCE);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);
	//end of transfer
	mock().expectOneCall("recvChar").andReturnValue(XModem::EOT);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	XModem modem(recvChar, sendChar);
	
	CHECK(true == modem.receive());

}
TEST(xmodemrcv, transfer_1_block_retransmit_crc)
{
	mock().setData("recvChar", 1);
	
	mock().expectNCalls(1, "sendChar").withParameter("sym", 'C');
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//crc - wrong
	mock().expectNCalls(1, "recvChar").andReturnValue(0x1C);
	mock().expectNCalls(1, "recvChar").andReturnValue(0xCD);
	//expect NACK	
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::NACK);

	//retry transfer!

	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//some crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0x1C);
	mock().expectNCalls(1, "recvChar").andReturnValue(0xCE);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);
	//end of transfer
	mock().expectOneCall("recvChar").andReturnValue(XModem::EOT);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	XModem modem(recvChar, sendChar);
	
	CHECK(true == modem.receive());

}
TEST(xmodemrcv, transfer_1_block_retransmit_chksum)
{
	mock().setData("recvChar", 1);
	
	mock().expectNCalls(16, "sendChar").withParameter("sym", 'C');
	mock().expectNCalls(16, "recvChar").andReturnValue(-1);

	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::NACK);
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//chksum
	mock().expectNCalls(1, "recvChar").andReturnValue(129);
	//expect NACK
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::NACK);

	//retry transfer!

	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//chksum
	mock().expectNCalls(1, "recvChar").andReturnValue(128);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);
	//end of transfer
	mock().expectOneCall("recvChar").andReturnValue(XModem::EOT);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	XModem modem(recvChar, sendChar);
	
	CHECK(true == modem.receive());

}
TEST(xmodemrcv, transfer_1_block_nack_limit)
{
	mock().setData("recvChar", 1);
	
	mock().expectNCalls(1, "sendChar").withParameter("sym", 'C');

	for (int i=0; i< 10; i++) {
		//set SOH
		mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
		//set block number =1 and ~block number = 254
		mock().expectOneCall("recvChar").andReturnValue(2);
		mock().expectOneCall("recvChar").andReturnValue(254);
		//wait nack
		mock().expectNCalls(1, "sendChar").
				withParameter("sym", XModem::NACK);
	}

	XModem modem(recvChar, sendChar);
	
	CHECK(false == modem.receive());

}
TEST(xmodemrcv, transfer_1_block_nack_limit2)
{
	mock().setData("recvChar", 1);
	mock().expectNCalls(1, "sendChar").withParameter("sym", 'C');

	//first packet with wrong chksum

	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//some crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0x1C);
	mock().expectNCalls(1, "recvChar").andReturnValue(0xCD); //wrong
	//nack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::NACK);
	//others 9 pakect with worng packet No
	for (int i=0; i< 9; i++) {
		//set SOH
		mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
		//set block number =1 and ~block number = 254
		mock().expectOneCall("recvChar").andReturnValue(2);
		mock().expectOneCall("recvChar").andReturnValue(254);
		//wait nack
		mock().expectNCalls(1, "sendChar").
				withParameter("sym", XModem::NACK);
	}

	XModem modem(recvChar, sendChar);
	
	CHECK(false == modem.receive());

}
TEST(xmodemrcv, transfer_3_block_same_blockNo)
{
	mock().setData("recvChar", 1);
	mock().setData("dataHandler", 1);
	
	mock().expectNCalls(1, "sendChar").withParameter("sym", 'C');
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0x1C);
	mock().expectNCalls(1, "recvChar").andReturnValue(0xCE);
	//check data
	mock().expectNCalls(1, "dataHandler").withParameter("no", 1).
					      withParameter("data", 'A').
					      withParameter("size", 128).
						andReturnValue(1);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	//second frame with blockNo 1. Should be discarded
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0x1C);
	mock().expectNCalls(1, "recvChar").andReturnValue(0xCE);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	//third frame wit blockNo 2. Should be discarded
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =2 and ~block number = 253
	mock().expectOneCall("recvChar").andReturnValue(2);
	mock().expectOneCall("recvChar").andReturnValue(253);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('B');
	//some crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0xDF);
	mock().expectNCalls(1, "recvChar").andReturnValue(0x8F);
	//check data
	mock().expectNCalls(1, "dataHandler").withParameter("no", 2).
					      withParameter("data", 'B').
					      withParameter("size", 128).
						andReturnValue(1);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	//end of transfer
	mock().expectOneCall("recvChar").andReturnValue(XModem::EOT);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	XModem modem(recvChar, sendChar, dataHandler);
	
	CHECK(true == modem.receive());

}

TEST(xmodemrcv, can_from_transmitter)
{
	mock().setData("recvChar", 1);
	
	mock().expectNCalls(1, "sendChar").withParameter("sym", 'C');
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0x1C);
	mock().expectNCalls(1, "recvChar").andReturnValue(0xCE);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	//send two CAN
	mock().expectNCalls(2, "recvChar").andReturnValue(XModem::CAN);
	//wait ACK
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	
	XModem modem(recvChar, sendChar, dataHandler);
	
	CHECK(false == modem.receive());
}
TEST(xmodemrcv, invalid_can_from_transmitter)
{
	mock().setData("recvChar", 1);
	
	mock().expectNCalls(1, "sendChar").withParameter("sym", 'C');
	//set SOH
	mock().expectOneCall("recvChar").andReturnValue(XModem::SOH);
	//set block number =1 and ~block number = 254
	mock().expectOneCall("recvChar").andReturnValue(1);
	mock().expectOneCall("recvChar").andReturnValue(254);
	//data
	mock().expectNCalls(128, "recvChar").andReturnValue('A');
	//crc
	mock().expectNCalls(1, "recvChar").andReturnValue(0x1C);
	mock().expectNCalls(1, "recvChar").andReturnValue(0xCE);
	//wait ack
	mock().expectNCalls(1, "sendChar").withParameter("sym", XModem::ACK);

	//send one CAN and one invalid symbol
	mock().expectNCalls(1, "recvChar").andReturnValue(XModem::CAN);
	mock().expectNCalls(1, "recvChar").andReturnValue(1);
	//wait 3 CANs
	mock().expectNCalls(3, "sendChar").withParameter("sym", XModem::CAN);

	
	XModem modem(recvChar, sendChar, dataHandler);
	
	CHECK(false == modem.receive());
}
TEST_GROUP(xmodemtrn)
{

	void setup()
	{
	}

	void teardown()
	{
		mock().checkExpectations();
		mock().clear();	
	}
	
};

TEST(xmodemtrn, expectCandNack)
{
	mock().setData("recvChar", 1);
	mock().expectNCalls(16, "recvChar").andReturnValue(-1);	
	mock().expectNCalls(16, "recvChar").andReturnValue(-1);	
	XModem modem(recvChar, sendChar, dataHandler);
	
	CHECK(false == modem.transmit());
}
TEST(xmodemtrn, sendFrame)
{
	mock().setData("recvChar", 1);
	mock().setData("dataHandler", 3);
	mock().setData("dataHandler_data", 'A');
	mock().expectOneCall("dataHandler").withParameter("no",1).
		withParameter("size", 128).andReturnValue(1);
	
	mock().expectOneCall("dataHandler").withParameter("no",2).
		withParameter("size", 128).andReturnValue(0);
							
	mock().expectNCalls(1, "recvChar").andReturnValue('C');
	//start of frame
	mock().expectOneCall("sendChar").withParameter("sym", XModem::SOH);
	//block No
	mock().expectOneCall("sendChar").withParameter("sym", 1);
	//inv block num - due to limitation of char use -2
	mock().expectOneCall("sendChar").withParameter("sym", -2);
	//data
	mock().expectNCalls(128, "sendChar").withParameter("sym", 'A');
	//crc
	mock().expectNCalls(1, "sendChar").withParameter("sym", 28);
	mock().expectNCalls(1, "sendChar").withParameter("sym", -50);
	//return ACK
	mock().expectNCalls(1, "recvChar").andReturnValue(XModem::ACK);
	//wait EOT
	mock().expectOneCall("sendChar").withParameter("sym", XModem::EOT);
	//return ACK
	mock().expectNCalls(1, "recvChar").andReturnValue(XModem::ACK);
	
	XModem modem(recvChar, sendChar, dataHandler);
	
	CHECK(true == modem.transmit());
}
TEST(xmodemtrn, sendTwoFrames)
{
	mock().setData("recvChar", 1);
	mock().setData("dataHandler", 3);
	mock().setData("dataHandler_data", 'A');

	mock().expectOneCall("dataHandler").withParameter("no",1).
		withParameter("size", 128).andReturnValue(1);
	
	mock().expectOneCall("dataHandler").withParameter("no",2).
		withParameter("size", 128).andReturnValue(1);
	
	mock().expectOneCall("dataHandler").withParameter("no",3).
		withParameter("size", 128).andReturnValue(0);
							
	mock().expectNCalls(1, "recvChar").andReturnValue('C');
	//start of frame
	mock().expectOneCall("sendChar").withParameter("sym", XModem::SOH);
	//block No
	mock().expectOneCall("sendChar").withParameter("sym", 1);
	//inv block num - due to limitation of char use -2
	mock().expectOneCall("sendChar").withParameter("sym", -2);
	//data
	mock().expectNCalls(128, "sendChar").withParameter("sym", 'A');
	//crc
	mock().expectNCalls(1, "sendChar").withParameter("sym", 28);
	mock().expectNCalls(1, "sendChar").withParameter("sym", -50);
	
	//return ACK
	mock().expectNCalls(1, "recvChar").andReturnValue(XModem::ACK);

	//start of frame
	mock().expectOneCall("sendChar").withParameter("sym", XModem::SOH);
	//block No
	mock().expectOneCall("sendChar").withParameter("sym", 2);
	//inv block num - due to limitation of char use -2
	mock().expectOneCall("sendChar").withParameter("sym", -3);
	//data
	mock().expectNCalls(128, "sendChar").withParameter("sym", 'A');
	//crc
	mock().expectNCalls(1, "sendChar").withParameter("sym", 28);
	mock().expectNCalls(1, "sendChar").withParameter("sym", -50);
	
	//return ACK
	mock().expectNCalls(1, "recvChar").andReturnValue(XModem::ACK);
	//wait EOT
	mock().expectOneCall("sendChar").withParameter("sym", XModem::EOT);
	//return ACK
	mock().expectNCalls(1, "recvChar").andReturnValue(XModem::ACK);
	
	XModem modem(recvChar, sendChar, dataHandler);
	
	CHECK(true == modem.transmit());
}
TEST(xmodemtrn, sendFrameCan)
{
	mock().setData("recvChar", 1);
	
	mock().setData("dataHandler", 3);
	mock().setData("dataHandler_data", 'A');


	mock().expectOneCall("dataHandler").withParameter("no",1).
		withParameter("size", 128).andReturnValue(1);
	
							
	mock().expectNCalls(1, "recvChar").andReturnValue('C');
	//start of frame
	mock().expectOneCall("sendChar").withParameter("sym", XModem::SOH);
	//block No
	mock().expectOneCall("sendChar").withParameter("sym", 1);
	//inv block num - due to limitation of char use -2
	mock().expectOneCall("sendChar").withParameter("sym", -2);
	//data
	mock().expectNCalls(128, "sendChar").withParameter("sym", 'A');
	//crc
	mock().expectNCalls(1, "sendChar").withParameter("sym", 28);
	mock().expectNCalls(1, "sendChar").withParameter("sym", -50);
	//return CAN
	mock().expectNCalls(1, "recvChar").andReturnValue(XModem::CAN);
	/*
	//return ACK
	mock().expectNCalls(1, "recvChar").andReturnValue(XModem::ACK);
	*/
	XModem modem(recvChar, sendChar, dataHandler);
	
	CHECK(false == modem.transmit());
}
