/* HTTPRangeDownload.cpp */
/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "HTTPRangeDownload.h"
#include "mbed.h"

HTTPRangeDownload::HTTPRangeDownload(char *buf, size_t startAddress, size_t stopAddress)
{
	rangeReset(buf, startAddress, stopAddress);
}

void HTTPRangeDownload::rangeReset(char *buf, size_t startAddress, size_t stopAddress){
	m_startAddress = startAddress;
	m_stopAddress = stopAddress;
	writeReset();
	m_buf = buf;
	memset(m_buf, 0xFF, (stopAddress - startAddress + 1));
}

size_t HTTPRangeDownload::getContentEnd(void){
	return m_contentEnd;
}

//IHTTPDataIn
/*virtual*/ void HTTPRangeDownload::writeReset()
{
	m_pos = 0;
}

/*virtual*/ int HTTPRangeDownload::write(const char* buf, size_t len)
{
	memcpy(m_buf+m_pos, buf , len); // store CHUNK in buffer
	m_pos += len;
    return OK;
}

/*virtual*/ void HTTPRangeDownload::setDataType(const char* type) //Internet media type from Content-Type header
{

}

/*virtual*/ void HTTPRangeDownload::setIsChunked(bool chunked) //From Transfer-Encoding header
{

}

/*virtual*/ void HTTPRangeDownload::setDataLen(size_t len) //From Content-Length header, or if the transfer is chunked, next chunk length
{
	m_dataLen = len; //Content Length
}

/*virtual*/ void HTTPRangeDownload::setContentEnd(size_t contentEnd)
{
	m_contentEnd = contentEnd; //Content Range: bytes x-y/z <- get z
}

//IHTTPData
/*virtual*/ bool HTTPRangeDownload::getHeader(char* header, size_t maxHeaderLen){
	//TODO:: more complex handling multiple headers
	char byteServingHeader[64];
	sprintf(byteServingHeader, "Range: bytes=%d-%d",m_startAddress,m_stopAddress);
	if (((sizeof(byteServingHeader)) > maxHeaderLen)) {
		// skip
		return false;
	}
	memcpy(header,byteServingHeader,sizeof(byteServingHeader));
	return true;
}
