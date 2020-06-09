#include "IPDebug.h"
#include "Timer.h"
#include "ImageList.h"

namespace cvlib
{

	static StringArray		m_log;
	static Timer			m_timer;
	static MatDescPtrList	m_debugArray;

	IPDebug::IPDebug()
	{
	}
	IPDebug::~IPDebug()
	{
		m_debugArray.release();
	}
	void IPDebug::getDebugInfoY(Mat& image, int nMaxRow)
	{
		if (nMaxRow == -1)
		{
			m_debugArray.getAppend(AxeY, AT_Left, image);
		}
		else
		{
			int k = 0;
			MatDescPtrList finalList;
			MatDescPtrList temp;
			for (int i = 0; i < m_debugArray.getSize(); i++)
			{
				if (k == nMaxRow)
				{
					k = 0;
					Mat subImage;
					temp.getAppend(AxeY, AT_Left, subImage);
					finalList.add(new Mat(subImage));
					temp.removeAll();
				}
				temp.add(m_debugArray.getAt(i));
				k++;
			}
			if (temp.getSize() != 0)
			{
				Mat subImage;
				temp.getAppend(AxeY, AT_Left, subImage);
				finalList.add(new Mat(subImage));
				temp.removeAll();
			}
			finalList.getAppend(AxeX, AT_Up, image);
		}
	}
	Mat* IPDebug::getDebugInfoY(int nMaxRow)
	{
		Mat image;
		getDebugInfoY(image, nMaxRow);
		return new Mat(image);
	}
	void IPDebug::getDebugInfoX(Mat& image, int nMaxCol)
	{
		if (nMaxCol == -1)
		{
			m_debugArray.getAppend(AxeX, AT_Up, image);
		}
		else
		{
			int k = 0;
			MatDescPtrList finalList;
			MatDescPtrList temp;
			for (int i = 0; i < m_debugArray.getSize(); i++)
			{
				if (k == nMaxCol)
				{
					k = 0;
					Mat subImage;
					temp.getAppend(AxeX, AT_Up, subImage);
					finalList.add(new Mat(subImage));
					temp.removeAll();
				}
				temp.add(m_debugArray.getAt(i));
				k++;
			}
			if (temp.getSize() != 0)
			{
				Mat subImage;
				temp.getAppend(AxeX, AT_Up, subImage);
				finalList.add(new Mat(subImage));
				temp.removeAll();
			}
			finalList.getAppend(AxeY, AT_Left, image);
		}
	}
	Mat* IPDebug::getDebugInfoX(int nMaxCol)
	{
		Mat image;
		getDebugInfoX(image, nMaxCol);
		return new Mat(image);
	}

	void IPDebug::resetDebug()
	{
		m_debugArray.release();
	}

	void IPDebug::addDebug(const Mat& m, const char* comment)
	{
		Mat matWithComment = makeImageWithComment(m, comment);
		m_debugArray.add(new Mat(matWithComment));
	}


	void IPDebug::resetLog()
	{
		m_log.removeAll();
		m_timer.reset();
		m_timer.resume();
	}
	void IPDebug::logging(const char* fmt, ...)
	{
		char buffer[5120];
		va_list args;
		va_start(args, fmt);
		vsprintf(buffer, fmt, args);
		va_end(args);
#if CVLIB_OS == CVLIB_OS_APPLE
		printf("%s", buffer);
		printf("\n");
#else
		m_log.add(buffer);
#endif
	}
	void IPDebug::loggingTime(const char* fmt, ...)
	{
		char buffer[512];
		va_list args;
		va_start(args, fmt);
		vsprintf(buffer, fmt, args);
		va_end(args);

		double rtime;
		rtime = m_timer.getElapsedTime() * 1000;

		size_t len = strlen(buffer);
		if (len < 500)
		{
			char sztemp[512];
			sprintf(sztemp, "%s %.8f ms", buffer, rtime);
			m_log.add(sztemp);
		}
		else
		{
			char* szdata = new char[len + 512];
			sprintf(szdata, "%s %.8f ms", buffer, rtime);
			m_log.add(szdata);
			delete[]szdata;
		}
		m_timer.resume();
	}

	void IPDebug::getLog(StringArray& log)
	{
		log.append(m_log);
	}

}