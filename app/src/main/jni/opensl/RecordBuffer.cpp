//
// Created by darrenyuan on 2022/10/1.
//

#include "RecordBuffer.h"

RecordBuffer::RecordBuffer(int bufferSize) {
    // 创建一个二维数组，里面有2个buffer，然后每次录音取出一个，录制好后再写入文件
    // 2个buffer依次来存储PCM数据
    buffer = new short *[2];
    for (int i = 0; i < 2; i++) {
        buffer[i] = new short[bufferSize];
    }
}

RecordBuffer::~RecordBuffer() {}

/**
 * 即将要录入PCM数据的buffer
 * @return
 */
short *RecordBuffer::getRecordBuffer() {
    index++;
    if (index > 1) {
        index = 0;
    }
    return buffer[index];
}

/**
 * 当前录制好的PCM数据的buffer
 * @return
 */
short *RecordBuffer::getNowBuffer() {
    return buffer[index];
}
