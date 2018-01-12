#include "StdAfx.h"
#include "Gim.h"


unsigned long CGim::BGRA2RGBA(unsigned long crColor)
{
	unsigned char data[4];
	data[0] = ((unsigned char *)&crColor)[2];
	data[1] = ((unsigned char *)&crColor)[1];
	data[2] = ((unsigned char *)&crColor)[0];
	data[3] = ((unsigned char *)&crColor)[3];

	return *(unsigned long *)data;
}


CGim::CGim(unsigned char *gim_data, unsigned long gim_size)
{
	m_gim_data = gim_data;
	m_gim_size = gim_size;

	gim();
}

CGim::~CGim(void)
{
}


void CGim::gim()
{
	unsigned char *gim_data_iter = m_gim_data;
	
	GIM_IMAGE gi;
	GIM_FILE_HEADER *_file_header = (GIM_FILE_HEADER *)gim_data_iter;
	gim_data_iter += sizeof(GIM_FILE_HEADER);

	//获得真实大小
	m_gim_size = sizeof(GIM_FILE_FLAGS) + (unsigned long)_file_header->gfeof.EOFAddr;

	GIM_COMMON_HEADER *_common_header = (GIM_COMMON_HEADER *)gim_data_iter;
	
	
	while (_common_header->flags != GIM_FILEINFO_DATA_PART_FLAG)
	{
		memset(&gi, 0, sizeof(GIM_IMAGE));
		do{
			//解析
			_common_header = (GIM_COMMON_HEADER *)gim_data_iter;
			switch (_common_header->flags)
			{
			case GIM_FILEINFO_ADDR_PART_FLAG:
				//文件头做起始
				if (gi.file_info)
				{
					m_vecimages.push_back(gi);
				}
				gi.file_info = (GIM_FILE_FILEINFO_ADDR_PART *)_common_header;
				break;
			case GIM_IMAGE_DATA_PART_FLAG:
				gi.image = (GIM_FILE_IMAGE_DATA_PART *)_common_header;
				break;
			case GIM_PALETTE_DATA_PART_FLAG:
				gi.palette = (GIM_FILE_PALETTE_DATA_PART *)_common_header;
				break;
			case GIM_FILEINFO_DATA_PART_FLAG:
				if (gi.file_info)
				{
					m_vecimages.push_back(gi);
				}
				return;
			default:
				if (gi.file_info)
				{
					m_vecimages.push_back(gi);
				}
				return;
			}

			gim_data_iter += _common_header->size1;
			if (gim_data_iter - m_gim_data >= m_gim_size)
			{
				if (gi.file_info)
				{
					m_vecimages.push_back(gi);
				}
				return;
			}
			if (_common_header->size1 == 0)
				break;

		}while (true);

		
	}

}