class Message:
    _data = []
    data_bytes = []

    def __init__(self, action = 0, config = 0):
        self.action = action
        self.size = self.calculate_size(config)
        self.config = config

    def set_data(self, data):
        self._data = data

    def decode_header(self, byte_array):
        self.action = bool.from_bytes(byte_array[:1], byteorder='big')
        self.size = int.from_bytes(byte_array[1:3], byteorder='big')
        self.config = int.from_bytes(byte_array[3:5], byteorder='big')

    def encode_message(self):
        return self.action.to_bytes(1, byteorder='big') + self.size.to_bytes(2, byteorder='big') + self.config.to_bytes(2, byteorder='big') + self.encode_data()

    #will use config info to determine the size of the data being sent
    #need to first determine what all data can be transferred 
    def calculate_size(self, config):
        return 64

    #will use config info to determine what data is being sent and encode it
    #need to first determine what all data can be transferred
    def encode_data(self):
        ex1 = 2354
        ex2 = 453462
        ex3 = 34523
        ex4 = 215241
        ex5 = 2354
        ex6 = 453462
        ex7 = 34523
        ex8 = 215241
        ex9 = 2354
        ex10 = 453462
        ex11 = 34523
        ex12 = 215241
        ex13 = 2354
        ex14 = 453462
        ex15 = 34523
        ex16 = 215241
        return ex1.to_bytes(4, byteorder='big') + ex2.to_bytes(4, byteorder='big') + ex3.to_bytes(4, byteorder='big') + ex4.to_bytes(4, byteorder='big') + ex5.to_bytes(4, byteorder='big') + ex6.to_bytes(4, byteorder='big') + ex7.to_bytes(4, byteorder='big') + ex8.to_bytes(4, byteorder='big') + ex9.to_bytes(4, byteorder='big') + ex10.to_bytes(4, byteorder='big') + ex11.to_bytes(4, byteorder='big') + ex12.to_bytes(4, byteorder='big') + ex13.to_bytes(4, byteorder='big') + ex14.to_bytes(4, byteorder='big') + ex15.to_bytes(4, byteorder='big') + ex16.to_bytes(4, byteorder='big')

    #use the config to determine what data is present   
    #need to first determine what all data can be transferred
    def decode_data(self):
        vals = []
        vals.append(int.from_bytes(self.data_bytes[:4], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[4:8], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[8:12], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[12:16], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[16:20], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[20:24], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[24:28], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[28:32], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[32:36], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[36:40], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[40:44], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[44:48], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[48:52], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[52:56], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[56:60], byteorder='big'))
        vals.append(int.from_bytes(self.data_bytes[60:64], byteorder='big'))
        return vals


    def to_string(self):
        return "Message Action {} with config {} has size {}".format(self.action, self.config, self.size)