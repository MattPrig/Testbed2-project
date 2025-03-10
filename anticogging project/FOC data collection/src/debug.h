#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

#define DEBUG_VERBOSE 4
#define DEBUG_NORMAL 3
#define DEBUG_WARNING 2
#define DEBUG_ERROR 1
#define DEBUG_NONE 0



#define DEBUG_LEVEL DEBUG_NORMAL

class Debug : public Stream{
    public:
        Debug(Stream* stream, uint8_t debug_level = DEBUG_NORMAL);

        void print(String x, int debug_level=DEBUG_NORMAL);
        void println(String x, int debug_level=DEBUG_NORMAL);

        void print(int x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};
        void println(int x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};

        void print(float x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};
        void println(float x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};

        void print(double x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};
        void println(double x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};

        void print(uint8_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};
        void println(uint8_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};

        void print(uint16_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};
        void println(uint16_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};

        void print(uint32_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};
        void println(uint32_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};

        void print(int8_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};
        void println(int8_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};

        void print(int16_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};
        void println(int16_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};

        void print(int32_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};
        void println(int32_t x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};

        void print(char x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};
        void println(char x, int debug_level=DEBUG_NORMAL){print((String)x, debug_level);};

        void println(){println("", debug_level);};
        void print(){print("", debug_level);};

        void setDebugLevel(uint8_t debug_level);

        int available(){return stream->available();};
        int read(){return stream->read();};
        int peek(){return stream->peek();};
        size_t readBytesUntil(char terminator, char *buffer, size_t length){return stream->readBytesUntil(terminator, buffer, length);};
        size_t readBytes(char *buffer, size_t length){return stream->readBytes(buffer, length);}; 
        void flush(){stream->flush();};
        size_t write(uint8_t c){return stream->write(c);};


    private:
        uint8_t debug_level;
        Stream* stream;
};


#endif