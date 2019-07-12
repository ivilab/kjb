
/* $Id: big_endian_and_shift.c 21664 2017-08-05 17:53:40Z kobus $ */


#include "l/l_incl.h" 

int main(void)
{
    unsigned int base = 0;
    unsigned int swapped = 0;
    unsigned int full_base = 0;
    unsigned int temp_full_base = 0;
    unsigned int restored = 0;

    unsigned int j = 0;
    unsigned int swapped_j = 0;
    unsigned int restored_j = 0;

    unsigned int edge_id = 1987;
    int restored_edge = 0;
    unsigned int base_edge = 0;

    int object_id = 0;
    int object_type = 0;
    int restored_object_id = 0;
    int restored_object_type = 0;

    int base_oid = 0;
    int base_otype = 0;

    if(! kjb_is_bigendian())
    {
        kjb_printf("Little endian\n");
        /*bswap_u32((uint32_t *) &(swapped));*/
    }
    else
    {
        kjb_printf("Big endian\n");
    }

    for(edge_id = 0; edge_id < 3000; edge_id++)
    {
        base_edge = edge_id<<16;
        for(object_type = 0; object_type < 4; object_type++)
        {
            base_oid = object_type<<14;
            for(object_id = 0; object_id < 32; object_id++)
            {
                base_otype = object_id<<8;
                for(base = 0; base < 32; base++)
                {
                    swapped = base;
                    if(! kjb_is_bigendian())
                    {
                        /*bswap_u32((uint32_t *) &(swapped));*/
                    }
                    temp_full_base = (edge_id<<16) | ((object_type<<14) | ((object_id<<8) | (swapped<<3)));
                    /*temp_full_base = (edge_id<<16) | ((object_type<<14) | ((swapped<<3)));*/
                    /*printf("shifted:%d\n", full_base);*/
                    for(j = 0; j < 8; j++)
                    {
                        swapped_j = j;
                        if(! kjb_is_bigendian())
                        {
                            /*bswap_u32((uint32_t *) &(swapped_j));*/
                        }
                        /*printf("Experiment:%d\n", full_base);*/
                        full_base = temp_full_base | swapped_j;
                        if(! kjb_is_bigendian())
                        {
                            /*bswap_u32((uint32_t *) &(full_base));*/
                        }
                        /*printf("Experiment:%d\n", full_base);*/
                        restored_j = full_base&0x07;
                        /*printf("Experiment:%d\n", restored_j);*/
                        /*restored = (full_base<<5) &(24&0xFF);*/
                        restored = (full_base>>3);
                        restored = restored&0x1f;
                        restored_edge = full_base>>16;
                        restored_object_type = full_base>>14;
                        restored_object_type = restored_object_type&0x03;
                        restored_object_id = full_base>>8;
                        restored_object_id = restored_object_id&0x1f;
                        /*printf("Experimentr:%d\n", restored_edge);*/
                        /*printf("reshifted:%d\n", restored);*/
                        if(! kjb_is_bigendian())
                        {
                            /*bswap_u32((uint32_t *) &(restored));
                            bswap_u32((uint32_t *) &(restored_j));*/
                        }
                        /*printf("%d\n", restored);
                        printf("%d\n", base);*/
                        ASSERT(j == restored_j);

                        ASSERT(base == restored);
                        ASSERT(restored_edge == (int)edge_id);
                        ASSERT(restored_object_type == object_type);
                        ASSERT(restored_object_id == object_id);
                    }
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
