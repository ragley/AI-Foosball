# CAN Messages
This document contains the details of the CAN messages used in the communication between *controllers* and the *command* in the AI Foosball project.

First a brief overview of CAN. CAN (Closed Area Network) is a wire communication protocol originally designed for communication between many embedded micro controllers in the automotive industry. CAN allows for multiple devices in parallel to communicate to each other over two wires at a max speed of 1Mbps. This allows for cross communication between multiple devices without a need for a routing device and without the need for excess wires.

CAN functions on the idea that each device talks at the same time unless a more important device talks over it. To achieve this all devices send at the same time but if a device tries to send a **1** while another device is sending a **0**, the device sending **1** stops sending and instead listens. This means that a **ID** of `0b00` has a higher priority than an **ID** of `0b10`. With this in mind, all the **IDs** have been chosen following this priority hierarchy.

Messages from the *command* will always have the highest priority so the first bit of every *command* message *ID* will always be **0**. The second bit will indicate if the message is a **stop** command or not. Because **stop** commands are the most important, a stop command will be represented by a **0** as the second bit. The following four bits indicate which *controller* the message is from or going to.

The ID will only utilize the final 7-bits of the IDs 11-bits. The message data length is always 8 bytes. All bit data is using **Big-endian**.

ID (7 bits)
| Bits | 0                    | 1                         | 2                             | 3               | 4               | 5               | 6               |
| :--: | :------------------: | :-----------------------: | :---------------------------: | :-------------: | :-------------: | :-------------: | :-------------: |
| `0`  | message from command | message from controllers  | stop\stopped message          | **NOT** pole 1  | **NOT** pole 2  | **NOT** pole 3  | **NOT** pole 4  |
| `1`  | message from table   | message from player poles | **NON** stop\stopped message  | pole 1          | pole 2          | pole 3          | pole 4          |

DATA (8 bytes)
| Bytes | 0-3               | 4-8          |
| :---: | :---------------: | :----------: |
| `X`   | Displacement Data | Angular Data |


|    ID     | Description                                   |
| :-------: | :-------------------------------------------: |
|`0b0XXXXX1`| Message from *command* to *controller* **1**  |
|`0b0XXXX1X`| Message from *command* to *controller* **2**  |
|`0b0XXX1XX`| Message from *command* to *controller* **3**  |
|`0b0XX1XXX`| Message from *command* to *controller* **4**  |
|`0b10XXXX1`| Message from *controller* **1** to *command*  |
|`0b10XXX1X`| Message from *controller* **2** to *command*  |
|`0b10XX1XX`| Message from *controller* **3** to *command*  |
|`0b10X1XXX`| Message from *controller* **4** to *command*  |
|`0b11XXXX1`| Message from *player pole* **1** to *command* |
|`0b11XXX1X`| Message from *player pole* **2** to *command* |
|`0b11XX1XX`| Message from *player pole* **3** to *command* |
|`0b11X1XXX`| Message from *player pole* **4** to *command* |


|    ID    | Displacement Data Length | Rotational Data Length | Description                           |
| :------: | :----------------------: | :-------------------:| :-------------------------------------: |
|`0b0X0XXXX`| **X**                   | **X**                | **Stop** command from *command*         |
|`0b100XXXX`| **X**                   | **X**                | **Stopped** message from *controller*   |
|`0b110XXXX`| **X**                   | **X**                | **Stopped** message from *player pole*  |
|`0b001XXXX`| **4 Bytes**             | **4 Bytes**          | **Move** command from *command*         |
|`0b101XXXX`| **4 Bytes**             | **4 Bytes**          | **Location** message from *controller*  |
|`0b111XXXX`| **4 Bytes**             | **4 Bytes**          | **Location** message from *player pole* |