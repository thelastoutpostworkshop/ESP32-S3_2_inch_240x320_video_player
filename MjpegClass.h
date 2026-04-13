/*******************************************************************************
 * JPEGDEC Wrapper Class
 *
 * Dependent libraries:
 * JPEGDEC: https://github.com/bitbank2/JPEGDEC.git
 ******************************************************************************/
#ifndef _MJPEGCLASS_H_
#define _MJPEGCLASS_H_

#define READ_BUFFER_SIZE 1024
#define MAXOUTPUTSIZE (MAX_BUFFERED_PIXELS / 16 / 16)

#include <FS.h>

#include <JPEGDEC.h>

class MjpegClass
{
public:
  bool setup(
      Stream *input, uint8_t *mjpeg_buf, JPEG_DRAW_CALLBACK *pfnDraw, bool useBigEndian,
      int x, int y, int widthLimit, int heightLimit)
  {
    _input = input;
    _mjpeg_buf = mjpeg_buf;
    _pfnDraw = pfnDraw;
    _useBigEndian = useBigEndian;
    _x = x;
    _y = y;
    _widthLimit = widthLimit;
    _heightLimit = heightLimit;
    _inputindex = 0;
    _scale = -1;

    if (!_read_buf)
    {
      _read_buf = static_cast<uint8_t *>(malloc(READ_BUFFER_SIZE));
    }

    return _read_buf != nullptr;
  }

  bool readMjpegBuf()
  {
    if (_inputindex == 0)
    {
      _buf_read = _input->readBytes(_read_buf, READ_BUFFER_SIZE);
      _inputindex += _buf_read;
    }
    _mjpeg_buf_offset = 0;
    int i = 0;
    bool found_FFD8 = false;
    while ((_buf_read > 0) && (!found_FFD8))
    {
      i = 0;
      while ((i < _buf_read) && (!found_FFD8))
      {
        if ((_read_buf[i] == 0xFF) && (_read_buf[i + 1] == 0xD8))
        {
          found_FFD8 = true;
        }
        ++i;
      }
      if (found_FFD8)
      {
        --i;
      }
      else
      {
        _buf_read = _input->readBytes(_read_buf, READ_BUFFER_SIZE);
      }
    }
    uint8_t *_p = _read_buf + i;
    _buf_read -= i;
    bool found_FFD9 = false;
    if (_buf_read > 0)
    {
      i = 3;
      while ((_buf_read > 0) && (!found_FFD9))
      {
        if ((_mjpeg_buf_offset > 0) && (_mjpeg_buf[_mjpeg_buf_offset - 1] == 0xFF) && (_p[0] == 0xD9))
        {
          found_FFD9 = true;
        }
        else
        {
          while ((i < _buf_read) && (!found_FFD9))
          {
            if ((_p[i] == 0xFF) && (_p[i + 1] == 0xD9))
            {
              found_FFD9 = true;
              ++i;
            }
            ++i;
          }
        }

        memcpy(_mjpeg_buf + _mjpeg_buf_offset, _p, i);
        _mjpeg_buf_offset += i;
        const size_t remaining = _buf_read - i;
        if (remaining > 0)
        {
          memcpy(_read_buf, _p + i, remaining);
          _buf_read = _input->readBytes(_read_buf + remaining, READ_BUFFER_SIZE - remaining);
          _p = _read_buf;
          _inputindex += _buf_read;
          _buf_read += remaining;
        }
        else
        {
          _buf_read = _input->readBytes(_read_buf, READ_BUFFER_SIZE);
          _p = _read_buf;
          _inputindex += _buf_read;
        }
        i = 0;
      }
      if (found_FFD9)
      {
        return true;
      }
    }

    return false;
  }

  bool drawJpg()
  {
    _remain = _mjpeg_buf_offset;
    _jpeg.openRAM(_mjpeg_buf, _remain, _pfnDraw);
    if (_scale == -1)
    {
      int max_mcus;
      int w = _jpeg.getWidth();
      int h = _jpeg.getHeight();
      const float ratio = static_cast<float>(h) / _heightLimit;
      if (ratio <= 1)
      {
        _scale = 0;
        max_mcus = _widthLimit / 16;
      }
      else if (ratio <= 2)
      {
        _scale = JPEG_SCALE_HALF;
        max_mcus = _widthLimit / 8;
        w /= 2;
        h /= 2;
      }
      else if (ratio <= 4)
      {
        _scale = JPEG_SCALE_QUARTER;
        max_mcus = _widthLimit / 4;
        w /= 4;
        h /= 4;
      }
      else
      {
        _scale = JPEG_SCALE_EIGHTH;
        max_mcus = _widthLimit / 2;
        w /= 8;
        h /= 8;
      }
      _jpeg.setMaxOutputSize(max_mcus);
      _x = (w > _widthLimit) ? 0 : ((_widthLimit - w) / 2);
      _y = (_heightLimit - h) / 2;
    }
    if (_useBigEndian)
    {
      _jpeg.setPixelType(RGB565_BIG_ENDIAN);
    }
    _jpeg.decode(_x, _y, _scale);
    _jpeg.close();

    return true;
  }

private:
  Stream *_input = nullptr;
  uint8_t *_mjpeg_buf = nullptr;
  JPEG_DRAW_CALLBACK *_pfnDraw = nullptr;
  bool _useBigEndian = false;
  int _x = 0;
  int _y = 0;
  int _widthLimit = 0;
  int _heightLimit = 0;

  uint8_t *_read_buf = nullptr;
  int32_t _mjpeg_buf_offset = 0;

  JPEGDEC _jpeg;
  int _scale = -1;

  int32_t _inputindex = 0;
  int32_t _buf_read = 0;
  int32_t _remain = 0;
};

#endif // _MJPEGCLASS_H_
