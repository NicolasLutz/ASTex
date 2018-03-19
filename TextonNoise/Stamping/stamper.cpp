#include "stamper.h"

namespace ASTex
{

namespace Stamping
{

Stamper::Stamper(const std::vector<Eigen::Vector2f> &pointArray, const ImageRGBd &tampon) :
    m_pointArray(pointArray), m_stamp(tampon)
{}

BombingStamper::BombingStamper(const std::vector<Eigen::Vector2f> &pointArray, const ImageRGBd &tampon) :
    Stamper(pointArray, tampon)
{}


ImageRGBd BombingStamper::generate(int imageWidth, int imageHeight)
{
    ImageRGBd im_out;

//    std::vector<Eigen::Vector2f> points;
//    points = m_pointArray->Generate(nb_points);

    im_out.initItk(imageWidth, imageHeight, true);

    for(std::vector<Eigen::Vector2f>::const_iterator it=m_pointArray.begin(); it!=m_pointArray.end(); ++it)
    {
        int i = im_out.width() * (*it)[0]; //i & j: single point coordinates in im_out
        int j = im_out.height() * (*it)[1];

        int textonWidth = m_stamp.width();
        int textonHeight = m_stamp.height();
        Region reg = gen_region(i-textonWidth/2, j-textonHeight/2, textonWidth, textonHeight); //the region we stamp
        im_out.for_region_pixels(reg, [&] (ImageRGBd::PixelType& pix, int x, int y)
        {
            if(x>0 && x<im_out.width() && y>0 && y<im_out.height())
            {
                int tx=x-i+textonWidth/2; //texton coordinate
                int ty=y-j+textonHeight/2; //texton coordinate
                pix = m_stamp.pixelAbsolute(tx, ty);
            }
        });
    }

    return im_out;
}

TextonStamper::TextonStamper(const std::vector<Eigen::Vector2f> &pointArray, const ImageRGBd &tampon) :
    Stamper(pointArray, tampon),
    m_ratioX(1.0),
    m_ratioY(1.0),
    m_periodicity(false),
    m_bilinearInterpolation(false),
    m_useMargins(true)
{}

ImageRGBd TextonStamper::generate(int imageWidth, int imageHeight)
{
    ImageRGBd im_out;
    double nbHit;
    double nbHitFullDomain;
    double textonWidth;
    double textonHeight;

    ImageRGBd::PixelType mean, sum;
    for(int i=0; i<3; ++i)
    {
        sum[i]=0;
        mean[i]=0;
    }
    m_stamp.for_all_pixels([&] (const ImageRGBd::PixelType &pix)
    {
        for(int i=0; i<3; ++i)
            mean[i] += pix[i];
    });
    //sum=mean;
    for(int i=0; i<3; ++i)
        mean[i]/=m_stamp.width()*m_stamp.height();

    m_stamp.for_all_pixels([&] (ImageRGBd::PixelType &pix)
    {
        for(int i=0; i<3; ++i)
        {
            pix[i] -= mean[i];
            pix[i] /= std::sqrt(m_stamp.width() * m_stamp.height());
            sum[i] += pix[i];
        }
    });

//    std::vector<Eigen::Vector2f> points;
//    points = m_pointArray->Generate(nb_points);

    im_out.initItk(imageWidth, imageHeight, true);

    textonWidth = m_stamp.width();
    textonHeight = m_stamp.height();

    nbHit=0;
    nbHitFullDomain = 0;

    for(std::vector<Eigen::Vector2f>::const_iterator it=m_pointArray.begin(); it!=m_pointArray.end(); ++it)
    {
//        int i = im_out.width() * (*it)[0]; //i & j: single point coordinates in im_out (double)
//        int j = im_out.height() * (*it)[1];

        double i, j;

        if(m_periodicity || (!m_periodicity && !m_useMargins))
        {
            i = imageWidth * (*it)[0];
            j = imageHeight * (*it)[1];
        }
        else
        {
            //we need to introduce margins here, to shoot textons outside of the domain
            i = (imageWidth + textonWidth ) * (*it)[0] - textonWidth/2.0;
            j = (imageHeight + textonHeight ) * (*it)[1] - textonHeight/2.0;
        }

        int otx=std::floor(i-textonWidth/2.0), oty=std::floor(j-textonHeight/2.0); //texton origin in texture space (top left)
        double dx= m_bilinearInterpolation ? (i-textonWidth/2.0)-otx : 0; //gap between texton and image pixels
        double dy= m_bilinearInterpolation ? (j-textonHeight/2.0)-oty : 0; //0 if no bilinear interpolation : equivalent to clamping dx and dy

        Region reg = gen_region(otx, oty, m_bilinearInterpolation ? textonWidth+1 : textonWidth, m_bilinearInterpolation ? textonHeight+1 : textonHeight);
        if(m_periodicity)
        {
            im_out.for_region_pixels(reg, [&] (ImageRGBd::PixelType& pix, int x, int y) //with periodicity
            {
                int tx=x-otx; //texton coordinate in texton space
                int ty=y-oty; //texton coordinate

                for(int c=0; c<3; ++c) //c: channel
                {
                    double interpolatedValue = 0.0;
                    if(m_bilinearInterpolation) //cond: speedup exploiting branch prediction, but not needed for correctness
                    {
                        if(tx > 0)
                            if(ty > 0)
                                interpolatedValue += (dx*dy)*m_stamp.pixelAbsolute(tx-1, ty-1)[c];
                            if(ty < textonHeight)
                                interpolatedValue += (dx*(1-dy))*m_stamp.pixelAbsolute(tx-1, ty)[c];
                        if(tx < textonWidth)
                            if(ty > 0)
                                interpolatedValue += ((1-dx)*dy)*m_stamp.pixelAbsolute(tx, ty-1)[c];
                            if(ty < textonHeight)
                                interpolatedValue += ((1-dx)*(1-dy))*m_stamp.pixelAbsolute(tx, ty)[c];
                    }
                    else
                        interpolatedValue += m_stamp.pixelAbsolute(tx, ty)[c];


                    im_out.pixelAbsolute((x+im_out.width())%imageWidth, (y+im_out.height())%imageHeight)[c] += interpolatedValue;
                }
            });
            nbHit=nbHit + textonWidth*textonHeight; //with periodicity, the entire energy of the texton hits the texture all the time
        }
        else
        {
            //the region we stamp : it is one pixel longer (per dim.) when the texton can stamped between pixels <=> when bilinear interpolation is activated
            im_out.for_region_pixels(reg, [&] (ImageRGBd::PixelType& pix, int x, int y) //without periodicity
            {
                if(x>=0 && y>=0 && x<imageWidth && y<imageHeight)
                {
                    int tx=x-otx; //texton coordinate
                    int ty=y-oty; //texton coordinate

                    double pHit3 = 0.0;

                    for(int c=0; c<3; ++c) //c: channel
                    {
                        double interpolatedValue = 0.0;
                        if(m_bilinearInterpolation) //cond: speedup exploiting branch prediction, but not needed for correctness
                        {
                            if(tx > 0)
                            {
                                if(ty > 0)
                                {
                                    interpolatedValue += (dx*dy)*m_stamp.pixelAbsolute(tx-1, ty-1)[c];
                                    pHit3 += (dx*dy);
                                }
                                if(ty < textonHeight)
                                {
                                    interpolatedValue += (dx*(1-dy))*m_stamp.pixelAbsolute(tx-1, ty)[c];
                                    pHit3 += (dx*(1-dy));
                                }
                            }
                            if(tx < textonWidth)
                            {
                                if(ty > 0)
                                {
                                    interpolatedValue += ((1-dx)*dy)*m_stamp.pixelAbsolute(tx, ty-1)[c];
                                    pHit3 += ((1-dx)*dy);
                                }
                                if(ty < textonHeight)
                                {
                                    interpolatedValue += ((1-dx)*(1-dy))*m_stamp.pixelAbsolute(tx, ty)[c];
                                    pHit3 += (1-dx)*(1-dy);
                                }
                            }
                        }
                        else
                        {
                            ++pHit3;
                            interpolatedValue += m_stamp.pixelAbsolute(tx, ty)[c];
                        }

                        pix[c] += interpolatedValue;
                    }

                    nbHit += pHit3/3.0;
                }
            });
            nbHitFullDomain += textonWidth*textonHeight;
        }
    }

    float nbHitPerPixel = nbHit/(imageWidth*imageHeight);
    float lambda = float(nbHitPerPixel)/(m_stamp.width() * m_stamp.height());

    std::cout << "Texton noise: number of impacts per pixel: " << std::to_string(nbHitPerPixel) << std::endl;

    im_out.for_all_pixels([&] (ImageRGBd::PixelType &pix) {
        for(int i=0; i<3; ++i)
            pix[i] = 1.0/sqrt(lambda) * (pix[i] - lambda * sum[i]) + mean[i];
    });

    return im_out;
}

//vérifier paramètres
//le tampon s'échantillonne
//mip-map => taille vs resolution

} //namsepace Stamping

} //namespace ASTex