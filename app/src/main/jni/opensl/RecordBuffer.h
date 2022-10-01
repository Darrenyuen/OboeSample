//
// Created by darrenyuan on 2022/10/1.
//

#ifndef OBOESAMPLE_RECORDBUFFER_H
#define OBOESAMPLE_RECORDBUFFER_H

class RecordBuffer {
public:
    short **buffer;
    int index = -1;
public:
    RecordBuffer(int bufferSize);
    ~RecordBuffer();

    /**
     * 得到一个新的录制buffer
     */
     short* getRecordBuffer();

     /**
      * 得到当前录制buffer
      */
      short* getNowBuffer();
};

#endif //OBOESAMPLE_RECORDBUFFER_H
