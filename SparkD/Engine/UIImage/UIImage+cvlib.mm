//
//  UIImage+OpenCV.mm
//  OpenCVClient
//
//  Created by Robin Summerhill on 02/09/2011.
//  Copyright 2011 Aptogo Limited. All rights reserved.
//
//  Permission is given to use this source code file without charge in any
//  project, commercial or otherwise, entirely at your risk, with the condition
//  that any redistribution (in part or whole) of source code must retain
//  this copyright and permission notice. Attribution in compiled projects is
//  appreciated but not required.
//

#import "UIImage+cvlib.h"

static void ProviderReleaseDataNOP(void *info, const void *data, size_t size)
{
    // Do not release memory
    return;
}



@implementation UIImage (UIImage_cvlib)

-(cvlib::Mat*)CVMat
{
    
    CGColorSpaceRef colorSpace = CGImageGetColorSpace(self.CGImage);
    
    CGFloat cols = self.size.width;
    CGFloat rows = self.size.height;
    int len=(int)(rows*cols)*4;
    unsigned char* pdata=new unsigned char[len];
    CGContextRef contextRef = CGBitmapContextCreate(pdata,                 // Pointer to backing data
                                                    cols,                      // Width of bitmap
                                                    rows,                     // Height of bitmap
                                                    8,                          // Bits per component
                                                    cols*4,              // Bytes per row
                                                    colorSpace,                 // Colorspace
                                                    kCGImageAlphaNoneSkipLast |
                                                    kCGBitmapByteOrderDefault); // Bitmap info flags
    //  kCGImageAlphaNone |
    //     kCGBitmapByteOrderDefault); // Bitmap info flags
    
    cvlib::Mat* cvMat=new cvlib::Mat(rows, cols, cvlib::MAT_Tbyte3); // 8 bits per component, 4 channels
    int k=0;
    for (int i=0; i< rows; i++)
    {
        for (int j=0, kk=0; j<cols; j++, kk+=3,k+=4)
        {
            unsigned char temp;

            temp=pdata[k+2];
            cvMat->data.ptr[i][kk+0]=temp; // R
            temp=pdata[k+1];
            cvMat->data.ptr[i][kk+1]=temp; // G
            temp=pdata[k+0];
            cvMat->data.ptr[i][kk+2]=temp; // B
            
            int t=0;
            t=pdata[k+2]+pdata[k+1]+pdata[k+2];
            if (t !=0)
            {
                t=0;
            }
        }
    }
    
    CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows), self.CGImage);
    CGContextRelease(contextRef);
    delete []pdata;
    
    return cvMat;
}
-(cvlib::Mat*)CVGrayscaleMat
{
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
    CGFloat cols = self.size.width;
    CGFloat rows = self.size.height;
    
    cvlib::Mat* cvMat = new cvlib::Mat(rows, cols, cvlib::MAT_Tbyte); // 8 bits per component, 1 channel
 
    CGContextRef contextRef = CGBitmapContextCreate(cvMat->data.ptr[0],                 // Pointer to backing data
                                                    cols,                      // Width of bitmap
                                                    rows,                     // Height of bitmap
                                                    8,                          // Bits per component
                                                    cols,              // Bytes per row
                                                    colorSpace,                 // Colorspace
                                                    kCGImageAlphaNone |
                                                    kCGBitmapByteOrderDefault); // Bitmap info flags
    
    CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows), self.CGImage);
    CGContextRelease(contextRef);
    CGColorSpaceRelease(colorSpace);
    
    return cvMat;
}

+ (UIImage *)imageWithCVMat:(const cvlib::Mat&)cvMat
{
    return [[UIImage alloc] initWithCVMat:cvMat];
}

- (id)initWithCVMat:(const cvlib::Mat&)cvMat
{
    NSData *data = [NSData dataWithBytes:cvMat.data.ptr[0] length:cvMat.channels() * cvMat.rows()*cvMat.cols()];
    
    CGColorSpaceRef colorSpace;
    
    if (cvMat.channels() == 1)
    {
        colorSpace = CGColorSpaceCreateDeviceGray();
    }
    else
    {
        colorSpace = CGColorSpaceCreateDeviceRGB();
    }
    
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((CFDataRef)data);
    
    CGImageRef imageRef = CGImageCreate(cvMat.cols(),                                     // Width
                                        cvMat.rows(),                                     // Height
                                        8,                                              // Bits per component
                                        8 * cvMat.channels(),                           // Bits per pixel
                                        cvMat.cols(),                                  // Bytes per row
                                        colorSpace,                                     // Colorspace
                                        kCGImageAlphaNone | kCGBitmapByteOrderDefault,  // Bitmap info flags
                                        provider,                                       // CGDataProviderRef
                                        NULL,                                           // Decode
                                        false,                                          // Should interpolate
                                        kCGRenderingIntentDefault);                     // Intent   
    
    self = [self initWithCGImage:imageRef];
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    
    return self;
}

void dataProviderReleaseCallback(void* info, const void* data, size_t size)
{
    free ((void*)data);
}

UIImage* Mat2UIImage(const cvlib::Mat& img)
{
    UIImage *newImage = nil;
    
    int width = img.cols();
	int height = img.rows();
    int cn = img.channels();
    cvlib::uchar *rawImageData = (cvlib::uchar*) malloc(height * width * 4);
    int nrOfColorComponents = 4; //RGBA
    int bitsPerColorComponent = 8;
    int rawImageDataLength = width * height * nrOfColorComponents;
    BOOL interpolateAndSmoothPixels = NO;
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;//kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGDataProviderRef dataProviderRef;
    CGColorSpaceRef colorSpaceRef;
    CGImageRef imageRef;
    
    if (cn==4)//BGRA
    {
        for(int i=0,kk=0;i<rawImageDataLength;i+=4, kk+=4 )
        {
            {
                rawImageData[i]  =img.data.ptr[0][kk+2];
                rawImageData[i+1]=img.data.ptr[0][kk+1];
                rawImageData[i+2]=img.data.ptr[0][kk+0];
                rawImageData[i+3]=img.data.ptr[0][kk+3];
            }
        }
    }
    else if (cn==3)
    {
        for(int i=0,kk=0;i<rawImageDataLength;i+=4, kk+=3 )
        {
            {
                rawImageData[i]  =img.data.ptr[0][kk+0];
                rawImageData[i+1]=img.data.ptr[0][kk+1];
                rawImageData[i+2]=img.data.ptr[0][kk+2];
                rawImageData[i+3]=255;
            }
        }
    }
    else if (cn==1)
    {
        for(int i=0,kk=0;i<rawImageDataLength;i+=4, kk++ )
        {
            {
                rawImageData[i]  =img.data.ptr[0][kk];
                rawImageData[i+1]=img.data.ptr[0][kk];
                rawImageData[i+2]=img.data.ptr[0][kk];
                rawImageData[i+3]=255;
            }
        }
    }

    @try
    {
        GLubyte *rawImageDataBuffer = rawImageData;
        dataProviderRef = CGDataProviderCreateWithData(NULL, rawImageDataBuffer, rawImageDataLength, dataProviderReleaseCallback);
        
        colorSpaceRef = CGColorSpaceCreateDeviceRGB();
        imageRef = CGImageCreate(width, height, bitsPerColorComponent, bitsPerColorComponent * nrOfColorComponents, width * nrOfColorComponents, colorSpaceRef, bitmapInfo, dataProviderRef, NULL, interpolateAndSmoothPixels, renderingIntent);
        newImage = [UIImage imageWithCGImage:imageRef scale:1.0f orientation:UIImageOrientationUp];
    }
    @finally
    {
        CGDataProviderRelease(dataProviderRef);
        CGColorSpaceRelease(colorSpaceRef);
        CGImageRelease(imageRef);
    }
    dataProviderRef = nil;
    return newImage;
}

/**
 * Create ARGB Bitmap context from CGImageRef.
 * @ param  inImage  [in]   image
 * @ return  ARGB bitmap context
 */
//-(CGContextRef) createARGBBitmapContextFromImage:(UIImage*) inImage
CGContextRef createARGBBitmapContextFromImage (UIImage* inImage)
{
	CGContextRef    context = NULL;
	CGColorSpaceRef colorSpace;
	void *          bitmapData;
	int             bitmapByteCount;
	int             bitmapBytesPerRow;
	size_t pixelsWide, pixelsHigh;
	
	// Get image width, height. We'll use the entire image.
	
	pixelsWide = inImage.size.width;
	pixelsHigh = inImage.size.height;
	
	// Declare the number of bytes per row. Each pixel in the bitmap in this
	// example is represented by 4 bytes; 8 bits each of red, green, blue, and
	// alpha.
	bitmapBytesPerRow   = (pixelsWide * 4);
	bitmapByteCount     = (bitmapBytesPerRow * pixelsHigh);
	
	// Use the generic RGB color space.
	colorSpace = CGColorSpaceCreateDeviceRGB();
	
	if (colorSpace == NULL)
	{
		fprintf(stderr, "Error allocating color space\n");
		return NULL;
	}
	
	// Allocate memory for image data. This is the destination in memory
	// where any drawing to the bitmap context will be rendered.
	bitmapData = malloc( bitmapByteCount );
	if (bitmapData == NULL)
	{
		fprintf (stderr, "Memory not allocated!");
		CGColorSpaceRelease( colorSpace );
		return NULL;
	}
	
	// Create the bitmap context. We want pre-multiplied ARGB, 8-bits
	// per component. Regardless of what the source image format is
	// (CMYK, Grayscale, and so on) it will be converted over to the format
	// specified here by CGBitmapContextCreate.
	context = CGBitmapContextCreate (bitmapData,
									 pixelsWide,
									 pixelsHigh,
									 8,      // bits per component
									 bitmapBytesPerRow,
									 colorSpace,
									 kCGImageAlphaPremultipliedFirst);
	if (context == NULL)
	{
		free (bitmapData);
		fprintf (stderr, "Context not created!");
	}
	
	// Make sure and release colorspace before returning
	CGColorSpaceRelease( colorSpace );
	
	return context;
}
static cvlib::Mat* UIImage2Mat(UIImage* Image)
{
    if(Image == NULL)
        return NULL;
	int iW =0, iH =0;
	float xVar, yVar;
	UIImageOrientation imageOrientation = Image.imageOrientation;
	
	CGImageRef inImage = Image.CGImage;
	// Create off screen bitmap context to draw the image into. Format ARGB is 4 bytes for each pixel: Alpa, Red, Green, Blue
//	CGContextRef cgctx = [self createARGBBitmapContextFromImage:Image];
	CGContextRef cgctx = createARGBBitmapContextFromImage (Image);
	if (cgctx == NULL) { return NULL; /* error */ }
	
    size_t w = CGBitmapContextGetWidth(cgctx);
    size_t h = CGBitmapContextGetHeight(cgctx);
    
    cvlib::Mat* pMat=new cvlib::Mat(h,w,cvlib::MAT_Tchar3);
	
	CGRect rect;
	
	switch (imageOrientation) {
		case UIImageOrientationDown:
		case UIImageOrientationDownMirrored:
			xVar = w / 2 * cos(M_PI) - h / 2 * sin(M_PI) - w / 2;
			yVar = w / 2 * sin(M_PI) + h / 2 * cos(M_PI) - h / 2;
			CGContextTranslateCTM(cgctx, -xVar, -yVar);
			CGContextRotateCTM(cgctx, M_PI);
			rect = CGRectMake(0, 0, w, h);
			break;
		case UIImageOrientationLeft:
		case UIImageOrientationLeftMirrored:
			xVar = h / 2 * cos(M_PI/2) - w / 2 * sin(M_PI/2) - w / 2;
			yVar = h / 2 * sin(M_PI/2) + w / 2 * cos(M_PI/2) - h/ 2;
			CGContextTranslateCTM(cgctx, -xVar, -yVar);
			CGContextRotateCTM(cgctx, M_PI / 2);
			rect = CGRectMake(0, 0, h, w);
			break;
		case UIImageOrientationRight:
		case UIImageOrientationRightMirrored:
			xVar = h / 2 * cos(-M_PI/2) - w / 2 * sin(-M_PI/2) - w / 2;
			yVar = h / 2 * sin(-M_PI/2) + w / 2 * cos(-M_PI/2) - h / 2;
			CGContextTranslateCTM(cgctx, -xVar, -yVar);
			CGContextRotateCTM(cgctx, -M_PI / 2);
			rect = CGRectMake(0, 0, h, w);
			break;
		case UIImageOrientationUp:
		case UIImageOrientationUpMirrored:
			rect = CGRectMake(0, 0, w, h);
			break;
		default:
			break;
	}
	
	// Draw the image to the bitmap context. Once we draw, the memory
	// allocated for the context for rendering will then contain the
	// raw image data in the specified color space.
	CGContextDrawImage(cgctx, rect, inImage);
	
	// Now we can get a pointer to the image data associated with the bitmap
	// context.
	unsigned char* data = (unsigned char *)CGBitmapContextGetData (cgctx);
	if (data != NULL)
	{
		//offset locates the pixel in the data from x,y.
		//4 for 4 bytes of data per pixel, w is width of one row of data.
		int offset =0;
		for(iH =0; iH < h; iH++)
		{
            cvlib::uchar* pdata=pMat->data.ptr[iH];
			for(iW =0; iW < w; iW++)
			{
//				pdata[iW] = data[offset]; //A
				pdata[iW*3+0] = data[offset+1]; //R
				pdata[iW*3+1] = data[offset+2]; //G
				pdata[iW*3+2] = data[offset+3]; //B
				offset += 4;
			}
		}
	}
	
	// When finished, release the context
	CGContextRelease(cgctx);
	// Free image data memory for the context
	if (data) { free(data); }

	return pMat;
}


@end
