
/* $Id: scan.c 21341 2017-03-26 03:45:12Z kobus $ */



#include "l/l_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    char line[ 200 ]; 
    kjb_uint64 uint64_res; 
    kjb_uint32 uint32_res; 
    kjb_uint16 uint16_res; 
    kjb_int64 int64_res; 
    kjb_int32 int32_res; 
    kjb_int16 int16_res; 
    int scan_res; 
    float float_res;
    double double_res; 
    int count = 0;


    while ((term_get_line("scan> ",line,200)) != EOF)
    {
        EPETE(set_low_level_scan_options("scan-nan", 
                                         (count % 2 == 0) ? "f" : "t"));    
        EPETE(set_low_level_scan_options("scan-inf", 
                                         (count % 2 == 0) ? "f" : "t"));    
        
        p_stderr("ss1u64:\n"); 
        EPE(scan_res = ss1u64(line, &uint64_res));  
        if (scan_res != ERROR) { dbu(uint64_res); }
        p_stderr("\n"); 

        p_stderr("ss1i64:\n"); 
        EPE(scan_res = ss1i64(line, &int64_res));  
        if (scan_res != ERROR) { dbi(int64_res); }
        p_stderr("\n"); 

        p_stderr("ss1pi64:\n"); 
        EPE(scan_res = ss1pi64(line, &int64_res));  
        if (scan_res != ERROR) { dbi(int64_res); }
        p_stderr("\n"); 

        p_stderr("ss1spi64:\n"); 
        EPE(scan_res = ss1spi64(line, &int64_res));  
        if (scan_res != ERROR) { dbi(int64_res); }
        p_stderr("\n"); 

        p_stderr("ss1pi64_2:\n"); 
        EPE(scan_res = ss1pi64_2(line, &int64_res));  
        if (scan_res != ERROR) { dbi(int64_res); }
        p_stderr("\n"); 

        p_stderr("ss1u32:\n"); 
        EPE(scan_res = ss1u32(line, &uint32_res));  
        if (scan_res != ERROR) { dbu(uint32_res); }
        p_stderr("\n"); 

        p_stderr("ss1i32:\n"); 
        EPE(scan_res = ss1i32(line, &int32_res));  
        if (scan_res != ERROR) { dbi(int32_res); }
        p_stderr("\n"); 

        p_stderr("ss1pi32:\n"); 
        EPE(scan_res = ss1pi32(line, &int32_res));  
        if (scan_res != ERROR) { dbi(int32_res); }
        p_stderr("\n"); 

        p_stderr("ss1spi32:\n"); 
        EPE(scan_res = ss1spi32(line, &int32_res));  
        if (scan_res != ERROR) { dbi(int32_res); }
        p_stderr("\n"); 

        p_stderr("ss1pi32_2:\n"); 
        EPE(scan_res = ss1pi32_2(line, &int32_res));  
        if (scan_res != ERROR) { dbi(int32_res); }
        p_stderr("\n"); 

        p_stderr("ss1u16:\n"); 
        EPE(scan_res = ss1u16(line, &uint16_res));  
        if (scan_res != ERROR) { dbu(uint16_res); }
        p_stderr("\n"); 

        p_stderr("ss1i16:\n"); 
        EPE(scan_res = ss1i16(line, &int16_res));  
        if (scan_res != ERROR) { dbi(int16_res); }
        p_stderr("\n"); 

        p_stderr("ss1pi16:\n"); 
        EPE(scan_res = ss1pi16(line, &int16_res));  
        if (scan_res != ERROR) { dbi(int16_res); }
        p_stderr("\n"); 

        p_stderr("ss1spi16:\n"); 
        EPE(scan_res = ss1spi16(line, &int16_res));  
        if (scan_res != ERROR) { dbi(int16_res); }
        p_stderr("\n"); 

        p_stderr("ss1pi16_2:\n"); 
        EPE(scan_res = ss1pi16_2(line, &int16_res));  
        if (scan_res != ERROR) { dbi(int16_res); }
        p_stderr("\n"); 

        p_stderr("ss1f:\n"); 
        EPE(scan_res = ss1f(line, &float_res));  
        if (scan_res != ERROR) { dbf(float_res); } 
        if (scan_res != ERROR) { dbe(float_res); } 
        p_stderr("\n"); 

        p_stderr("ss1snf:\n"); 
        EPE(scan_res = ss1snf(line, &float_res));  
        if (scan_res != ERROR) { dbf(float_res); }
        if (scan_res != ERROR) { dbe(float_res); } 
        p_stderr("\n"); 

        p_stderr("ss1d:\n"); 
        EPE(scan_res = ss1d(line, &double_res));  
        if (scan_res != ERROR) { dbf(double_res); } 
        if (scan_res != ERROR) { dbe(double_res); } 
        p_stderr("\n"); 

        p_stderr("ss1snd:\n"); 
        EPE(scan_res = ss1snd(line, &double_res));  
        if (scan_res != ERROR) { dbf(double_res); } 
        if (scan_res != ERROR) { dbe(double_res); } 
        p_stderr("\n"); 

        count++;
    }

    return EXIT_SUCCESS; 
}

