#pragma once

#include <vector>
#include "cvlibbase/Inc/_cvlibbase.h"

namespace cvlib
{

// ----------------------------------------------------------------------------------------

const static int STRIP_PART_NOT_PRESENT=0x7FFFFFFF;

// ----------------------------------------------------------------------------------------

class CVLIB_DECLSPEC full_strip_detection
{
public:
	full_strip_detection(const Range& rect_,const std::vector<int>& parts_) : rect(rect_), parts(parts_) {}
	full_strip_detection(const full_strip_detection& t){rect = t.rect;parts = t.parts;}

	full_strip_detection(){}

    explicit full_strip_detection(const Range& rect_) : rect(rect_) {}

    const Range& get_rect() const { return rect; }
	Range& get_rect() { return rect; }
    int num_parts() const { return (int)parts.size(); }

    const int& part(int idx) const 
    { 
        // make sure requires clause is not broken
        assert(idx < num_parts());
        return parts[idx]; 
    }

	int& part(int idx)
    { 
        // make sure requires clause is not broken
        assert(idx < num_parts());
        return parts[idx]; 
    }

	void drawTo(Mat& image, COLOR color, int thickness = 1) const;
    void toFile (XFile* pfile);
    void fromFile (XFile* pfile);

public:
    Range rect;
    std::vector<int> parts;  
};

}