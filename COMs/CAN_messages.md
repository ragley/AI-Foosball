# CAN Messages
This document contains the details of the CAN messages used in the communication between *controllers* and the *AI* in the AI Foosball project.

First a brief overview of CAN. CAN (Closed Area Network) is a wire communication protocol originally designed for communication between many embedded micro controllers in the automotive industry. CAN allows for multiple devices in parallel to communicate to each other over two wires at a max speed of 1Mbps. This allows for cross communication between multiple devices without a need for a routing device and without the need for excess wires.

CAN functions on the idea that each device talks at the same time unless a more important device talks over it. To achieve this all devices send at the same time but if a device tries to send a **1** while another device is sending a **0**, the device sending **1** stops sending and instead listens. This means that a **ID** of `0b00` has a higher priority than an **ID** of `0b10`. With this in mind, all the **IDs** have been chosen following this priority hierarchy.

Messages from the *AI* will always have the highest priority so the first bit of every *AI* message *ID* will always be **0**. The second bit will indicate if the message is a **stop** command or not. Because **stop** commands are the most important, a stop command will be represented by a **0** as the second bit. The following four bits indicate which *controller* the message is from or going to.

The ID will only utilize the final 7-bits of the IDs 11-bits. The message data length is always 8 bytes. All bit data is using **Big-endian** meaning the bits on the left will be sent or received before the bits on the right.

**ID**: 7 bits

| **Bits**  | **0-1**                                                           | **2**                         | **3**            | **4**         | **5**         | **6**         |
| :-------: | :---------------------------------------------------------------: | :---------------------------: | :--------------: | :-----------: | :-----------: | :-----------: |
| `0`       | (`00`) message from *AI*         : (`01`) Zero command            | stop\stopped message          | **NOT** goal rod | **NOT** 2 rod | **NOT** 5 rod | **NOT** 3 rod |
| `1`       | (`11`) message from player poles : (`10`) message from controllers| **NON** stop\stopped message  | goal rod         | 2 rod         | 5 rod         | 3 rod         |

**DATA**: 8 bytes

| **Bytes** | **0-3**           | **4-8**      |
| :-------: | :---------------: | :----------: |
| `X`       | Displacement Data | Angular Data |


|    ID     | Description                                      |
| :-------: | :----------------------------------------------: |
|`0b00XXXX1`| Message from *AI* to *controller 3 rod* **1**    |
|`0b00XXX1X`| Message from *AI* to *controller 5 rod* **2**    |
|`0b00XX1XX`| Message from *AI* to *controller 2 rod* **3**    |
|`0b00X1XXX`| Message from *AI* to *controller goal rod* **4** |
|`0b01XXXX1`| Zero Message to *controller 3 rod* **1**         |
|`0b01XXX1X`| Zero Message to *controller 5 rod* **2**         |
|`0b01XX1XX`| Zero Message to *controller 2 rod* **3**         |
|`0b01X1XXX`| Zero Message to *controller goal rod* **4** |
|`0b10XXXX1`| Message from *controller 3 rod* to *AI*          |
|`0b10XXX1X`| Message from *controller 5 rod* to *AI*          |
|`0b10XX1XX`| Message from *controller 2 rod* to *AI*          |
|`0b10X1XXX`| Message from *controller goal rod* **4** to *AI* |
|`0b11XXXX1`| Message from *player 3 rod* **1** to *AI*        |
|`0b11XXX1X`| Message from *player 5 rod* **2** to *AI*        |
|`0b11XX1XX`| Message from *player 2 rod* **3** to *AI*        |
|`0b11X1XXX`| Message from *player goal rod* **4** to *AI*     |


|    ID     | Displacement Data Length | Rotational Data Length | Description                            |
| :-------: | :----------------------: | :---------------------:| :------------------------------------: |
|`0b0X0XXXX`| **X**                    | **X**                  | **Stop** command from *AI*             |
|`0b100XXXX`| **X**                    | **X**                  | **Stopped** message from *controller*  |
|`0b110XXXX`| **X**                    | **X**                  | **Stopped** message from *player*      |
|`0b001XXXX`| **4 Bytes**              | **4 Bytes**            | **Move** command from *AI*             |
|`0b101XXXX`| **4 Bytes**              | **4 Bytes**            | **Location** message from *controller* |
|`0b111XXXX`| **4 Bytes**              | **4 Bytes**            | **Location** message from *player*     |