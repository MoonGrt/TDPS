#include "mbed.h"

// Create a UnbufferedSerial object with a default baud rate.
static UnbufferedSerial pc(USBTX, USBRX);  // tx, rx
static UnbufferedSerial device(D1, D0);    // tx, rx
PwmOut myled(D3);

void send_go()
{
    char c[5] = {0xfa, 0xf1, 0x22, 0x33, 0x44};
    device.write(&c, 5);
}

bool send_flag = 0;
void rx_irp1()
{
    char c;

    // Read the data to clear the receive interrupt.
    if (pc.read(&c, 1)) {
        // Echo the input back to the terminal.
        pc.write(&c, 1);
        // send_go();
        // send_flag = 1;
    }
}

void rx_irp2()
{
    char c;
    // Read the data to clear the receive interrupt.
    if (device.read(&c, 1)) {
        // Echo the input back to the terminal.
        pc.write(&c, 1);
        if(c==0x44)
            send_flag = 1;
    }
}

int main(void)
{
    char c[3];
    char cnt = 2;
    myled.period(0.02f); //20ms
    myled.write(0.025);

    // Set desired properties (9600-8-N-1).
    pc.baud(115200);
    pc.format(
        /* bits */ 8,
        /* parity */ SerialBase::None,
        /* stop bit */ 1
    );
    // pc.attach(&rx_irp1, SerialBase::RxIrq);

    // Set desired properties (9600-8-N-1).
    device.baud(9600);
    device.format(
        /* bits */ 8,
        /* parity */ SerialBase::None,
        /* stop bit */ 1
    );
    // Register a callback to process a Rx (receive) interrupt.
    device.attach(&rx_irp2, SerialBase::RxIrq);

    while(1)
    {
        if(send_flag == 1)
        {
            myled.write(0.075);
            thread_sleep_for(2000);
            send_go();
            send_flag = 0;
            thread_sleep_for(2000);
            myled.write(0.025);
        }
    }
}
