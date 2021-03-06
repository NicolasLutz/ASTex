/*******************************************************************************
* ASTex:                                                                       *
* Copyright (C) IGG Group, ICube, University of Strasbourg, France             *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: https://astex-icube.github.io                                      *
* Contact information: astex@icube.unistra.fr                                  *
*                                                                              *
*******************************************************************************/



#ifndef __ASTEX_IMAGE_GRAY__
#define __ASTEX_IMAGE_GRAY__

#include "itkImage.h"
#include <ASTex/image_common.h>

namespace ASTex
{

template<typename CHANNEL_TYPE>
class ImageGrayBase : public ImageBase
{
protected:
	typename itk::Image< CHANNEL_TYPE >::Pointer itk_img_;

public:

	typedef itk::Image< CHANNEL_TYPE >				       ItkImg;
	typedef itk::ImageRegionIteratorWithIndex<ItkImg>      IteratorIndexed;
	typedef itk::ImageRegionIterator<ItkImg>               Iterator;
	typedef itk::ImageRegionConstIteratorWithIndex<ItkImg> ConstIteratorIndexed;
	typedef itk::ImageRegionConstIterator<ItkImg>          ConstIterator;


	static const uint32_t NB_CHANNELS = 1;
	typedef CHANNEL_TYPE PixelType;
	typedef double DoublePixelType;
	typedef CHANNEL_TYPE ASTexPixelType;
	typedef CHANNEL_TYPE DataType;

	ImageGrayBase():
		itk_img_(NULL)
	{}


	ImageGrayBase(typename itk::Image< CHANNEL_TYPE >::Pointer itk_im):
		itk_img_(itk_im)
	{}

protected:

	inline PixelType* getPixelsPtr()
	{
		return this->itk_img_->GetPixelContainer()->GetBufferPointer();
	}

	inline DataType* getDataPtr()
	{
		return getPixelsPtr();
	}

	inline const PixelType* getPixelsPtr() const
	{
		return this->itk_img_->GetPixelContainer()->GetBufferPointer();
	}

	inline const DataType* getDataPtr() const
	{
		return getPixelsPtr();
	}
};


template <class T> using ImageGray = ImageCommon< ImageGrayBase< T >, false >;
using ImageGrayu8  = ImageGray< uint8_t >;
using ImageGray8   = ImageGray< int8_t >;
using ImageGrayu16 = ImageGray< uint16_t >;
using ImageGray16  = ImageGray< int16_t >;
using ImageGrayu32 = ImageGray< uint32_t >;
using ImageGray32  = ImageGray< int32_t >;
using ImageGrayu64 = ImageGray< uint64_t >;
using ImageGray64  = ImageGray< int64_t >;
using ImageGrayf   = ImageGray< float >;
using ImageGrayd   = ImageGray< double >;
using ImageGraycf  = ImageGray< std::complex<float> >;
using ImageGraycd  = ImageGray< std::complex<double> >;


// ConstImages are Images that only support const operations, they are not modifiable.
// But they could be construct from Itk::Image::ConstPointer -> usefull in Filter writing.

template <class T> using ConstImageGray = ImageCommon< ImageGrayBase< T >, true >;
using ConstImageGrayu8  = ConstImageGray< uint8_t >;
using ConstImageGray8   = ConstImageGray< int8_t >;
using ConstImageGrayu16 = ConstImageGray< uint16_t >;
using ConstImageGray16  = ConstImageGray< int16_t >;
using ConstImageGrayu32 = ConstImageGray< uint32_t >;
using ConstImageGray32  = ConstImageGray< int32_t >;
using ConstImageGrayu64 = ConstImageGray< uint64_t >;
using ConstImageGray64  = ConstImageGray< int64_t >;
using ConstImageGrayf   = ConstImageGray< float >;
using ConstImageGrayd   = ConstImageGray< double >;
using ConstImageGraycf  = ConstImageGray< std::complex<float> >;
using ConstImageGraycd  = ConstImageGray< std::complex<double> >;


class ImagePixelCompactIndex : public ImageGrayu32
{
public:
	inline static uint32_t pos_to_ui32(const Index& p) { return p[0] | p[1]<<16;}
	inline static  Index ui32_to_pos(uint32_t idx) { return gen_index(idx & 0xffff,idx >> 16);}
	inline Index iget(int x, int y) { return ui32_to_pos(this->pixelAbsolute(x,y));	}
	inline void iset(int x, int y, int a, int b) {	this->pixelAbsolute(x,y) = a | b<<16;}
	inline void iset(int x, int y, const Index& p)	{ this->pixelAbsolute(x,y) = pos_to_ui32(p);}
};

}

#endif



