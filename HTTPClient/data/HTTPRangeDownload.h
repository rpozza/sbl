/* HTTPRangeDownload.h */
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


#ifndef HTTP_RANGE_DOWNLOAD_H_
#define HTTP_RANGE_DOWNLOAD_H_

#define OK 							0

#include "../IHTTPData.h"

/** A data endpoint to download data directly to the mbed internal flash memory
*/
class HTTPRangeDownload : public IHTTPDataIn
{
public:
  /** Create an HTTP instance for range download
   * @param startAddress : range beginning
   * @param stopAddress : end of range
   */
  HTTPRangeDownload(char *buf, size_t startAddress, size_t stopAddress);

  void rangeReset(char *buf, size_t startAddress, size_t stopAddress);

  size_t getContentEnd(void);

 protected:
  //IHTTPDataIn
  virtual void writeReset();
  
  virtual int write(const char* buf, size_t len);

  virtual void setDataType(const char* type); //Internet media type from Content-Type header

  virtual void setIsChunked(bool chunked); //From Transfer-Encoding header

  virtual void setDataLen(size_t len); //From Content-Length header, or if the transfer is chunked, next chunk length

  virtual bool getHeader(char* header, size_t maxHeaderLen);

  virtual void setContentEnd(size_t contentEnd);

private:
  size_t m_dataLen, m_pos, m_contentEnd, m_startAddress, m_stopAddress;
  char * m_buf;
};

#endif /* HTTPTEXT_H_ */
