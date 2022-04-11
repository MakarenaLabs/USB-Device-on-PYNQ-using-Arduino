#include "xparameters.h"
#include "xil_types.h"

#include "xiic.h"
#include "xintc.h"

#define MAILBOX_CMD_ADDR       (*(volatile u32 *)(0x0000FFFC))
#define MAILBOX_DATA(x)        (*(volatile u32 *)(0x0000F000 +((x)*4)))
#define MAILBOX_DATA_PTR(x)    ( (volatile u32 *)(0x0000F000 +((x)*4)))
#define MAILBOX_DATA_FLOAT(x)     (*(volatile float *)(0x0000F000 +((x)*4)))
#define MAILBOX_DATA_FLOAT_PTR(x) ( (volatile float *)(0x0000F000 +((x)*4)))

#define TEST 	0x69
#define SEND	0x9
#define INIT 	0x23


#define IIC_DEVICE_ID		XPAR_IIC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define IIC_INTR_ID			XPAR_INTC_0_IIC_0_VEC_ID

/*
 * The following constant defines the address of the IIC
 * temperature sensor device on the IIC bus. Note that since
 * the address is only 7 bits, this  constant is the address divided by 2.
 */
#define ARDUINO_ADDRESS	0x4 /* The actual address is 0x4 */

#define SEND_COUNT	16
#define RECEIVE_COUNT   16

XIic Iic;		  /* The instance of the IIC device */
XIic_Config *ConfigPtr;
XIntc InterruptController;

u8 WriteBuffer[SEND_COUNT];	/* Write buffer for writing a page. */
u8 ReadBuffer[RECEIVE_COUNT];	/* Read buffer for reading a page. */

volatile u8 TransmitComplete;
volatile u8 ReceiveComplete;


static int SetupInterruptSystem(XIic *IicInstPtr)
{
	int Status;

	if (InterruptController.IsStarted == XIL_COMPONENT_IS_STARTED) {
		return XST_SUCCESS;
	}

	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 *  the specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&InterruptController, IIC_INTR_ID,
				   (XInterruptHandler) XIic_InterruptHandler,
				   IicInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts.
	 */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupts for the IIC device.
	 */
	XIntc_Enable(&InterruptController, IIC_INTR_ID);

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				 (Xil_ExceptionHandler) XIntc_InterruptHandler,
				 &InterruptController);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
static void SendHandler(XIic *InstancePtr)
{
	TransmitComplete = 0;
}
static void ReceiveHandler(XIic *InstancePtr)
{
	ReceiveComplete = 0;
}
static void StatusHandler(XIic *InstancePtr, int Event)
{

}

int IicInit(u16 IicDeviceId, u8 ArduinoAddress){

	int Status;

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIic_LookupConfig(IicDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&Iic, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = SetupInterruptSystem(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Set the Transmit, Receive and Status handlers.
	 */
	XIic_SetSendHandler(&Iic, &Iic,
				(XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&Iic, &Iic,
				(XIic_Handler) ReceiveHandler);
	XIic_SetStatusHandler(&Iic, &Iic,
				  (XIic_StatusHandler) StatusHandler);

	Status = XIic_SetAddress(&Iic, XII_ADDR_TO_SEND_TYPE, ArduinoAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return Status;

}

int SendArduinoIIC(u16 IicDeviceId, u8 ArduinoAddress, u8 *ArduinoPtr, u8* TxData, u16 TxLength)
{
	int Status;
	u8 Index;

	for (Index = 0; Index < SEND_COUNT; Index++) {
		WriteBuffer[Index] = 0;
		ReadBuffer[Index] = 0;
	}

	TransmitComplete = 1;
	Iic.Options = 0x0;


	Status = XIic_Start(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//	IicInstance.Options = XII_REPEATED_START_OPTION;

	Status = XIic_MasterSend(&Iic, TxData, TxLength);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till data is transmitted.
	 */
	while ((TransmitComplete) || (XIic_IsIicBusy(&Iic) == TRUE)) {

	}


	Status = XIic_Stop(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int main(void)
{
	int Status;
	int cmd;
	u8 data[16] = {0};
	u8 data_len = 1;

	u8 ArduinoPtr;
	u8 ArduinoAddress = 0;


	while (1) {
        while((MAILBOX_CMD_ADDR & 0x01)==0);
        cmd = MAILBOX_CMD_ADDR;

        switch(cmd){
			case TEST:
				MAILBOX_DATA(0) = 69;
				MAILBOX_CMD_ADDR = 0x0;
				break;

			case INIT:
				ArduinoAddress = (u16) MAILBOX_DATA(0);
				IicInit(IIC_DEVICE_ID, ArduinoAddress);
				MAILBOX_CMD_ADDR = 0x0;
				break;

			case SEND:
				data[0] = (u16) MAILBOX_DATA(0);
				data_len = (u16) MAILBOX_DATA(1);
				Status =  SendArduinoIIC(IIC_DEVICE_ID, ArduinoAddress, &ArduinoPtr, data, data_len);
				MAILBOX_DATA(0) = (u16) Status;
				MAILBOX_CMD_ADDR = 0x0;
				break;

			default:
				MAILBOX_CMD_ADDR = 0x0;
				break;
        }
	}
	return 0;
}
