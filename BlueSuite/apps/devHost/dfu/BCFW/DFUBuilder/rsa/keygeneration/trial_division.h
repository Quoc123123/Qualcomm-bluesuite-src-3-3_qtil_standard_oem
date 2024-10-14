/**********************************************************************
 *
 *  trial_division.h
 *  
 *  Copyright (c) 2001-2017 Qualcomm Technologies International, Ltd.
 *  All Rights Reserved.
 *  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 *  
 *  Contains prime tables.
 *
 **********************************************************************/

#ifndef __TRIAL_DIVISION_H__
#define __TRIAL_DIVISION_H__

/* The prime_table[] array contains the prime numbers between 5 and 1009 */

const dword prime_table[PSIZE] =

{7  , 11 , 13 , 19 , 23 , 29 , 31 , 37 , 41 , 43 , 47 , 53 , 59 , 61 , 67 ,
 71 , 73 , 79 , 83 , 89 , 97 , 101, 103, 107, 109, 113, 127, 131, 137, 139,
 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227,
 229, 233, 239, 241, 251, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313,
 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499,
 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691,
 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907,
 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009};



/* The matrix prime_factor[][] contains the values of B^i mod p[j], where B = 2^32, i = 1,2, ...31 and the p[j]'s are */
/* the prime numbers between 5 and 1009. These values are used for inner-multiplying the 32-bit digits of a mutiple   */
/* precision number N which is up to 32 digits long (each 32 bits long). This is a very fast and efficient way to see */
/* if any of the primes p[j] in the table prime_table[] divides N (making N a composite number). This is the trial    */
/* division by odd primes < K (K = 1010), the first step in the Miller-Rabin probabilistic primality test. Note that  */
/* these constants can be used only for 32-bit digits, a different digit size will require different constants.       */

/*const word primefactor[PSIZE][PORDER32b] =*/
PrimeOrder32b primefactor =
{
{4,   2,   1,   4,   2,   1,   4,   2,   1,   4,   2,   1,   4,   2,   1,   4,   2,   1,   4,   2,   1,   4,   2,   1,   4,   2,   1,   4,   2,   1,   4  },
{4,   5,   9,   3,   1,   4,   5,   9,   3,   1,   4,   5,   9,   3,   1,   4,   5,   9,   3,   1,   4,   5,   9,   3,   1,   4,   5,   9,   3,   1,   4  },
{9,   3,   1,   9,   3,   1,   9,   3,   1,   9,   3,   1,   9,   3,   1,   9,   3,   1,   9,   3,   1,   9,   3,   1,   9,   3,   1,   9,   3,   1,   9  },
{6,   17,  7,   4,   5,   11,  9,   16,  1,   6,   17,  7,   4,   5,   11,  9,   16,  1,   6,   17,  7,   4,   5,   11,  9,   16,  1,   6,   17,  7,   4  },
{12,  6,   3,   13,  18,  9,   16,  8,   4,   2,   1,   12,  6,   3,   13,  18,  9,   16,  8,   4,   2,   1,   12,  6,   3,   13,  18,  9,   16,  8,   4  },
{16,  24,  7,   25,  23,  20,  1,   16,  24,  7,   25,  23,  20,  1,   16,  24,  7,   25,  23,  20,  1,   16,  24,  7,   25,  23,  20,  1,   16,  24,  7  },
{4,   16,  2,   8,   1,   4,   16,  2,   8,   1,   4,   16,  2,   8,   1,   4,   16,  2,   8,   1,   4,   16,  2,   8,   1,   4,   16,  2,   8,   1,   4  },
{7,   12,  10,  33,  9,   26,  34,  16,  1,   7,   12,  10,  33,  9,   26,  34,  16,  1,   7,   12,  10,  33,  9,   26,  34,  16,  1,   7,   12,  10,  33 },
{37,  16,  18,  10,  1,   37,  16,  18,  10,  1,   37,  16,  18,  10,  1,   37,  16,  18,  10,  1,   37,  16,  18,  10,  1,   37,  16,  18,  10,  1,   37 },
{16,  41,  11,  4,   21,  35,  1,   16,  41,  11,  4,   21,  35,  1,   16,  41,  11,  4,   21,  35,  1,   16,  41,  11,  4,   21,  35,  1,   16,  41,  11 },
{42,  25,  16,  14,  24,  21,  36,  8,   7,   12,  34,  18,  4,   27,  6,   17,  9,   2,   37,  3,   32,  28,  1,   42,  25,  16,  14,  24,  21,  36,  8  },
{42,  15,  47,  13,  16,  36,  28,  10,  49,  44,  46,  24,  1,   42,  15,  47,  13,  16,  36,  28,  10,  49,  44,  46,  24,  1,   42,  15,  47,  13,  16 },
{51,  5,   19,  25,  36,  7,   3,   35,  15,  57,  16,  49,  21,  9,   46,  45,  53,  48,  29,  4,   27,  20,  17,  41,  26,  28,  12,  22,  1,   51,  5  },
{57,  16,  58,  12,  13,  9,   25,  22,  34,  47,  56,  20,  42,  15,  1,   57,  16,  58,  12,  13,  9,   25,  22,  34,  47,  56,  20,  42,  15,  1,   57 },
{33,  17,  25,  21,  23,  22,  56,  39,  14,  60,  37,  15,  26,  54,  40,  47,  10,  62,  36,  49,  9,   29,  19,  24,  55,  6,   64,  35,  16,  59,  4  },
{9,   10,  19,  29,  48,  6,   54,  60,  43,  32,  4,   36,  40,  5,   45,  50,  24,  3,   27,  30,  57,  16,  2,   18,  20,  38,  58,  25,  12,  37,  49 },
{32,  2,   64,  4,   55,  8,   37,  16,  1,   32,  2,   64,  4,   55,  8,   37,  16,  1,   32,  2,   64,  4,   55,  8,   37,  16,  1,   32,  2,   64,  4  },
{50,  51,  22,  73,  16,  10,  26,  36,  62,  19,  2,   21,  23,  44,  67,  32,  20,  52,  72,  45,  38,  4,   42,  46,  9,   55,  64,  40,  25,  65,  11 },
{77,  36,  33,  51,  26,  10,  23,  28,  81,  12,  11,  17,  64,  31,  63,  37,  27,  4,   59,  61,  49,  38,  21,  40,  9,   29,  75,  48,  44,  68,  7  },
{45,  67,  78,  39,  64,  32,  16,  8,   4,   2,   1,   45,  67,  78,  39,  64,  32,  16,  8,   4,   2,   1,   45,  67,  78,  39,  64,  32,  16,  8,   4  },
{35,  61,  1,   35,  61,  1,   35,  61,  1,   35,  61,  1,   35,  61,  1,   35,  61,  1,   35,  61,  1,   35,  61,  1,   35,  61,  1,   35,  61,  1,   35 },
{68,  79,  19,  80,  87,  58,  5,   37,  92,  95,  97,  31,  88,  25,  84,  56,  71,  81,  54,  36,  24,  16,  78,  52,  1,   68,  79,  19,  80,  87,  58 },
{63,  55,  66,  38,  25,  30,  36,  2,   23,  7,   29,  76,  50,  60,  72,  4,   46,  14,  58,  49,  100, 17,  41,  8,   92,  28,  13,  98,  97,  34,  82 },
{29,  92,  100, 11,  105, 49,  30,  14,  85,  4,   9,   47,  79,  44,  99,  89,  13,  56,  19,  16,  36,  81,  102, 69,  75,  35,  52,  10,  76,  64,  37 },
{75,  66,  45,  105, 27,  63,  38,  16,  1,   75,  66,  45,  105, 27,  63,  38,  16,  1,   75,  66,  45,  105, 27,  63,  38,  16,  1,   75,  66,  45,  105},
{16,  30,  28,  109, 49,  106, 1,   16,  30,  28,  109, 49,  106, 1,   16,  30,  28,  109, 49,  106, 1,   16,  30,  28,  109, 49,  106, 1,   16,  30,  28 },
{16,  2,   32,  4,   64,  8,   1,   16,  2,   32,  4,   64,  8,   1,   16,  2,   32,  4,   64,  8,   1,   16,  2,   32,  4,   64,  8,   1,   16,  2,   32 },
{117, 65,  7,   33,  62,  49,  100, 41,  81,  45,  25,  43,  53,  44,  39,  109, 46,  11,  108, 60,  77,  101, 27,  15,  52,  58,  105, 102, 13,  80,  59 },
{34,  60,  122, 38,  59,  88,  115, 74,  50,  56,  123, 72,  119, 73,  16,  133, 1,   34,  60,  122, 38,  59,  88,  115, 74,  50,  56,  123, 72,  119, 73 },
{41,  13,  116, 30,  118, 112, 5,   66,  65,  24,  11,  34,  4,   25,  52,  47,  120, 55,  31,  20,  125, 121, 96,  44,  136, 16,  100, 69,  49,  63,  81 },
{129, 102, 46,  123, 73,  30,  145, 80,  39,  114, 104, 6,   29,  16,  127, 142, 140, 31,  125, 33,  85,  88,  28,  36,  25,  96,  17,  107, 95,  37,  5  },
{4,   16,  64,  105, 118, 19,  76,  2,   8,   32,  128, 59,  85,  38,  1,   4,   16,  64,  105, 118, 19,  76,  2,   8,   32,  128, 59,  85,  38,  1,   4  },
{93,  14,  46,  39,  16,  75,  67,  108, 153, 99,  101, 130, 1,   93,  14,  46,  39,  16,  75,  67,  108, 153, 99,  101, 130, 1,   93,  14,  46,  39,  16 },
{100, 57,  158, 152, 41,  25,  55,  121, 38,  51,  47,  136, 71,  91,  135, 134, 34,  140, 145, 156, 115, 90,  35,  77,  39,  151, 104, 131, 60,  132, 160},
{7,   49,  9,   63,  107, 81,  66,  128, 61,  93,  150, 48,  2,   14,  98,  18,  126, 47,  162, 132, 89,  122, 19,  133, 96,  4,   28,  29,  36,  85,  94 },
{96,  47,  14,  133, 139, 23,  132, 43,  149, 118, 83,  10,  95,  124, 140, 119, 6,   57,  109, 84,  106, 142, 138, 100, 85,  29,  16,  152, 60,  51,  52 },
{126, 124, 51,  161, 59,  95,  156, 145, 12,  80,  56,  75,  142, 171, 66,  82,  129, 144, 65,  135, 5,   93,  83,  76,  89,  116, 117, 64,  9,   60,  42 },
{15,  44,  117, 126, 80,  114, 81,  129, 125, 65,  70,  145, 3,   45,  132, 170, 16,  59,  161, 62,  25,  13,  14,  29,  73,  9,   135, 34,  148, 48,  177},
{147, 26,  2,   103, 52,  4,   15,  104, 8,   30,  17,  16,  60,  34,  32,  120, 68,  64,  49,  136, 128, 98,  81,  65,  5,   162, 130, 10,  133, 69,  20 },
{108, 84,  1,   108, 84,  1,   108, 84,  1,   108, 84,  1,   108, 84,  1,   108, 84,  1,   108, 84,  1,   108, 84,  1,   108, 84,  1,   108, 84,  1,   108},
{88,  61,  49,  175, 34,  37,  104, 90,  40,  171, 76,  187, 105, 178, 101, 23,  54,  24,  142, 85,  191, 63,  28,  100, 132, 190, 172, 164, 51,  154, 156},
{46,  126, 25,  155, 165, 28,  94,  145, 103, 161, 43,  187, 45,  80,  98,  130, 10,  62,  66,  51,  157, 58,  81,  144, 57,  35,  18,  32,  79,  52,  4  },
{51,  69,  143, 119, 161, 193, 137, 24,  169, 179, 56,  113, 66,  201, 123, 154, 47,  76,  78,  180, 107, 182, 209, 109, 73,  136, 184, 100, 36,  148, 163},
{7,   49,  120, 171, 82,  128, 4,   28,  196, 34,  15,  105, 66,  16,  112, 115, 136, 60,  197, 41,  64,  2,   14,  98,  17,  119, 164, 33,  8,   56,  169},
{176, 104, 144, 147, 221, 79,  57,  44,  26,  36,  207, 112, 190, 71,  11,  120, 9,   222, 28,  161, 188, 173, 30,  59,  169, 7,   97,  47,  100, 121, 185},
{161, 44,  214, 104, 27,  225, 43,  53,  60,  42,  121, 16,  57,  17,  218, 61,  203, 165, 1,   161, 44,  214, 104, 27,  225, 43,  53,  60,  42,  121, 16 },
{8,   64,  46,  135, 148, 19,  152, 51,  175, 2,   16,  128, 92,  37,  63,  38,  71,  102, 117, 4,   32,  23,  184, 74,  126, 76,  142, 204, 1,   8,   64 },
{110, 150, 9,   34,  155, 81,  67,  200, 12,  125, 127, 108, 169, 187, 16,  87,  10,  144, 66,  90,  101, 116, 93,  192, 88,  120, 55,  75,  124, 17,  197},
{15,  225, 1,   15,  225, 1,   15,  225, 1,   15,  225, 1,   15,  225, 1,   15,  225, 1,   15,  225, 1,   15,  225, 1,   15,  225, 1,   15,  225, 1,   15 },
{123, 69,  204, 243, 20,  201, 125, 64,  91,  149, 4,   241, 25,  63,  219, 80,  51,  249, 5,   113, 94,  16,  211, 100, 1,   123, 69,  204, 243, 20,  201},
{34,  104, 117, 33,  70,  13,  179, 37,  206, 166, 121, 169, 223, 218, 48,  54,  258, 93,  6,   204, 98,  176, 198, 157, 78,  22,  222, 184, 207, 200, 225},
{47,  57,  258, 21,  180, 121, 38,  172, 14,  120, 260, 115, 25,  99,  80,  263, 256, 196, 66,  143, 265, 81,  41,  44,  185, 87,  54,  117, 119, 213, 58 },
{219, 265, 41,  36,  25,  55,  121, 212, 87,  83,  20,  44,  151, 7,   178, 229, 16,  252, 175, 114, 34,  129, 67,  39,  140, 37,  244, 49,  162, 248, 112},
{27,  175, 16,  155, 30,  256, 264, 203, 218, 69,  201, 164, 273, 169, 131, 213, 211, 157, 84,  52,  19,  236, 1,   27,  175, 16,  155, 30,  256, 264, 203},
{35,  101, 163, 85,  165, 155, 86,  200, 256, 249, 4,   140, 123, 90,  59,  98,  58,  63,  238, 181, 153, 16,  279, 211, 79,  236, 111, 232, 252, 109, 162},
{250, 240, 4,   151, 111, 16,  38,  161, 64,  152, 78,  256, 42,  29,  175, 168, 116, 134, 106, 181, 253, 141, 158, 163, 281, 66,  86,  275, 264, 61,  251},
{133, 109, 140, 161, 24,  262, 272, 137, 55,  283, 135, 82,  65,  148, 53,  17,  210, 95,  36,  100, 115, 59,  229, 278, 56,  123, 244, 222, 226, 172, 22 },
{149, 97,  24,  199, 179, 269, 171, 305, 9,   113, 259, 216, 256, 76,  272, 4,   289, 81,  96,  182, 102, 155, 70,  299, 36,  145, 115, 250, 103, 304, 167},
{72,  208, 48,  35,  32,  127, 125, 292, 187, 91,  21,  268, 14,  75,  113, 50,  179, 137, 223, 195, 45,  130, 30,  294, 20,  196, 117, 27,  78,  18,  52 },
{76,  142, 150, 132, 16,  277, 81,  209, 234, 256, 50,  44,  214, 301, 27,  174, 78,  294, 121, 119, 280, 309, 9,   58,  26,  98,  249, 144, 302, 103, 3  },
{232, 251, 221, 235, 313, 23,  264, 67,  11,  16,  225, 212, 49,  273, 253, 51,  103, 121, 176, 256, 113, 222, 150, 247, 244, 182, 63,  34,  280, 292, 223},
{4,   16,  64,  256, 31,  124, 165, 329, 323, 299, 203, 150, 269, 83,  1,   4,   16,  64,  256, 31,  124, 165, 329, 323, 299, 203, 150, 269, 83,  1,   4  },
{26,  2,   52,  4,   104, 8,   208, 16,  79,  32,  158, 64,  316, 128, 295, 256, 253, 175, 169, 13,  1,   26,  2,   52,  4,   104, 8,   208, 16,  79,  32 },
{127, 167, 42,  129, 74,  29,  213, 332, 177, 271, 64,  147, 278, 259, 275, 225, 121, 99,  81,  224, 341, 279, 39,  95,  267, 250, 173, 110, 90,  326, 109},
{192, 219, 168, 148, 147, 304, 85,  266, 118, 320, 16,  280, 14,  245, 274, 258, 327, 313, 68,  143, 234, 256, 292, 224, 81,  196, 289, 346, 122, 41,  194},
{58,  187, 256, 22,  217, 231, 337, 131, 185, 140, 1,   58,  187, 256, 22,  217, 231, 337, 131, 185, 140, 1,   58,  187, 256, 22,  217, 231, 337, 131, 185},
{73,  303, 220, 264, 245, 294, 281, 50,  60,  72,  230, 276, 44,  340, 49,  346, 128, 10,  12,  158, 46,  127, 296, 68,  297, 141, 241, 2,   146, 247, 81 },
{60,  297, 204, 129, 33,  145, 259, 126, 220, 355, 14,  106, 121, 287, 338, 95,  195, 323, 296, 144, 199, 196, 16,  226, 348, 328, 229, 161, 118, 107, 181},
{235, 21,  86,  68,  314, 309, 253, 148, 91,  124, 46,  366, 220, 226, 144, 270, 40,  75,  94,  83,  109, 251, 51,  49,  325, 283, 111, 348, 93,  221, 88 },
{203, 277, 139, 171, 224, 371, 271, 58,  25,  148, 103, 64,  106, 294, 179, 332, 313, 246, 289, 301, 84,  376, 149, 306, 341, 245, 86,  24,  324, 205, 304},
{317, 143, 137, 150, 58,  2,   251, 286, 274, 300, 116, 4,   119, 189, 165, 217, 232, 8,   238, 378, 330, 51,  81,  16,  93,  373, 277, 102, 162, 32,  186},
{13,  169, 252, 164, 187, 97,  94,  55,  326, 348, 245, 73,  171, 278, 113, 302, 36,  79,  249, 125, 69,  119, 380, 272, 35,  66,  80,  262, 294, 321, 283},
{167, 99,  256, 273, 333, 31,  16,  290, 393, 126, 1,   167, 99,  256, 273, 333, 31,  16,  290, 393, 126, 1,   167, 99,  256, 273, 333, 31,  16,  290, 393},
{255, 63,  25,  360, 372, 224, 178, 77,  387, 39,  321, 51,  173, 5,   72,  315, 125, 196, 256, 318, 88,  385, 331, 195, 1,   255, 63,  25,  360, 372, 224},
{218, 80,  262, 265, 101, 341, 309, 286, 180, 385, 85,  125, 256, 184, 30,  405, 355, 89,  179, 167, 5,   272, 400, 83,  98,  96,  69,  318, 203, 82,  289},
{254, 409, 393, 100, 260, 257, 333, 363, 22,  141, 199, 266, 105, 273, 207, 203, 25,  65,  169, 188, 405, 215, 140, 364, 276, 131, 173, 366, 365, 111, 121},
{234, 26,  190, 255, 309, 315, 35,  191, 68,  335, 84,  290, 79,  383, 370, 275, 358, 414, 46,  239, 354, 320, 363, 321, 176, 347, 366, 181, 254, 75,  289},
{145, 337, 162, 216, 288, 384, 81,  108, 144, 192, 256, 54,  72,  96,  128, 27,  36,  48,  64,  229, 18,  24,  32,  330, 9,   12,  16,  165, 220, 6,   8  },
{27,  296, 198, 150, 153, 234, 256, 417, 1,   27,  296, 198, 150, 153, 234, 256, 417, 1,   27,  296, 198, 150, 153, 234, 256, 417, 1,   27,  296, 198, 150},
{260, 433, 196, 36,  141, 223, 32,  418, 247, 126, 274, 122, 112, 146, 206, 2,   81,  427, 392, 72,  282, 7,   64,  397, 55,  252, 109, 244, 224, 292, 412},
{341, 215, 220, 153, 342, 113, 435, 373, 52,  12,  105, 365, 425, 64,  117, 27,  347, 46,  181, 144, 374, 393, 227, 325, 75,  324, 177, 109, 400, 399, 58 },
{324, 359, 25,  18,  444, 176, 1,   324, 359, 25,  18,  444, 176, 1,   324, 359, 25,  18,  444, 176, 1,   324, 359, 25,  18,  444, 176, 1,   324, 359, 25 },
{407, 215, 218, 68,  256, 453, 200, 54,  42,  185, 347, 16,  114, 241, 289, 174, 440, 393, 1,   407, 215, 218, 68,  256, 453, 200, 54,  42,  185, 347, 16 },
{405, 370, 25,  444, 30,  164, 36,  289, 412, 439, 310, 158, 372, 374, 262, 80,  130, 96,  156, 23,  95,  212, 114, 70,  229, 84,  367, 193, 256, 416, 215},
{115, 261, 383, 60,  418, 381, 293, 359, 78,  173, 449, 242, 50,  194, 86,  167, 222, 65,  67,  297, 356, 196, 316, 226, 62,  185, 440, 133, 16,  451, 9  },
{52,  369, 41,  264, 185, 280, 83,  113, 272, 134, 430, 411, 357, 351, 39,  160, 381, 198, 22,  210, 179, 435, 204, 334, 89,  425, 151, 380, 146, 120, 169},
{384, 403, 35,  28,  214, 267, 22,  305, 244, 291, 137, 397, 126, 5,   4,   99,  175, 140, 112, 377, 110, 88,  262, 18,  206, 69,  151, 25,  20,  16,  396},
{338, 286, 242, 467, 58,  124, 30,  400, 301, 442, 374, 279, 311, 413, 312, 264, 111, 19,  91,  77,  215, 107, 128, 408, 83,  295, 362, 119, 288, 431, 65 },
{279, 263, 218, 429, 378, 388, 232, 407, 132, 3,   346, 298, 163, 305, 152, 182, 205, 239, 396, 9,   56,  403, 489, 424, 456, 55,  124, 226, 206, 27,  168},
{444, 31,  291, 462, 39,  350, 211, 371, 54,  24,  177, 245, 497, 110, 437, 416, 74,  421, 298, 77,  256, 391, 451, 145, 9,   4,   279, 124, 166, 351, 156},
{190, 387, 92,  378, 394, 416, 69,  32,  44,  312, 429, 24,  33,  234, 196, 18,  402, 427, 147, 265, 50,  446, 236, 73,  289, 83,  177, 432, 91,  188, 7  },
{355, 302, 320, 93,  439, 91,  238, 505, 107, 319, 247, 137, 280, 145, 66,  16,  81,  251, 30,  470, 407, 438, 245, 445, 185, 14,  389, 156, 408, 284, 38 },
{117, 143, 59,  130, 101, 355, 376, 228, 105, 302, 427, 464, 104, 185, 284, 405, 495, 84,  450, 29,  267, 500, 148, 123, 324, 396, 484, 360, 440, 422, 400},
{294, 141, 137, 7,   489, 464, 436, 49,  285, 110, 437, 343, 426, 247, 444, 309, 367, 160, 493, 71,  477, 74,  313, 497, 201, 518, 99,  341, 361, 488, 170},
{215, 240, 205, 254, 510, 368, 134, 137, 241, 420, 494, 174, 81,  103, 505, 375, 16,  194, 53,  34,  277, 45,  478, 521, 28,  69,  228, 330, 79,  214, 25 },
{423, 60,  218, 318, 499, 482, 402, 476, 52,  116, 385, 396, 126, 239, 449, 118, 137, 516, 15,  328, 353, 535, 394, 374, 119, 13,  29,  233, 99,  305, 470},
{452, 442, 378, 414, 533, 292, 532, 397, 90,  19,  233, 43,  498, 68,  101, 535, 82,  302, 39,  361, 528, 260, 550, 178, 248, 139, 444, 168, 184, 175, 6  },
{188, 438, 146, 424, 329, 485, 537, 179, 435, 145, 236, 454, 339, 113, 413, 513, 171, 57,  19,  194, 440, 522, 174, 58,  207, 69,  23,  383, 503, 543, 181},
{528, 543, 497, 107, 165, 63,  262, 69,  16,  482, 153, 555, 5,   364, 439, 209, 535, 256, 315, 172, 345, 80,  134, 196, 499, 25,  113, 488, 476, 399, 142},
{82,  443, 353, 396, 496, 131, 464, 362, 563, 486, 453, 31,  258, 29,  94,  285, 530, 64,  109, 373, 323, 220, 339, 390, 4,   328, 59,  270, 442, 271, 524},
{287, 435, 213, 546, 335, 363, 321, 384, 1,   287, 435, 213, 546, 335, 363, 321, 384, 1,   287, 435, 213, 546, 335, 363, 321, 384, 1,   287, 435, 213, 546},
{413, 339, 301, 456, 488, 203, 485, 138, 55,  409, 448, 119, 426, 425, 12,  260, 546, 90,  189, 573, 88,  537, 482, 73,  212, 93,  254, 416, 404, 144, 185},
{535, 399, 578, 277, 538, 225, 589, 232, 183, 60,  78,  220, 286, 16,  258, 454, 353, 281, 306, 42,  529, 154, 556, 367, 62,  555, 425, 256, 570, 148, 311},
{125, 51,  385, 205, 467, 272, 456, 95,  494, 53,  36,  307, 39,  83,  192, 40,  208, 243, 425, 413, 111, 98,  270, 206, 592, 323, 242, 300, 362, 325, 492},
{128, 157, 263, 8,   423, 54,  301, 64,  379, 432, 4,   512, 27,  451, 32,  490, 216, 2,   256, 314, 526, 16,  245, 108, 1,   128, 157, 263, 8,   423, 54 },
{400, 359, 348, 197, 497, 311, 572, 568, 182, 567, 389, 208, 41,  11,  151, 307, 186, 346, 4,   386, 222, 178, 181, 167, 30,  467, 451, 121, 447, 342, 225},
{573, 374, 365, 112, 424, 204, 422, 284, 287, 167, 63,  545, 268, 314, 313, 353, 592, 227, 115, 304, 100, 291, 7,   333, 166, 103, 171, 516, 202, 502, 149},
{63,  267, 162, 334, 64,  330, 429, 496, 398, 394, 142, 308, 277, 175, 536, 450, 585, 452, 94,  369, 418, 420, 546, 463, 170, 221, 349, 392, 16,  391, 570},
{513, 94,  559, 170, 550, 505, 323, 426, 31,  428, 438, 616, 318, 337, 180, 109, 207, 342, 269, 579, 526, 573, 543, 9,   284, 227, 79,  292, 617, 212, 431},
{172, 558, 64,  281, 376, 310, 316, 86,  279, 32,  456, 188, 155, 158, 43,  455, 16,  228, 94,  393, 79,  337, 543, 8,   114, 47,  512, 355, 484, 587, 4  },
{640, 1,   640, 1,   640, 1,   640, 1,   640, 1,   640, 1,   640, 1,   640, 1,   640, 1,   640, 1,   640, 1,   640, 1,   640, 1,   640, 1,   640, 1,   640},
{571, 40,  335, 314, 540, 343, 381, 217, 451, 321, 36,  623, 154, 486, 373, 150, 131, 213, 96,  161, 625, 10,  566, 400, 135, 568, 256, 215, 595, 241, 9  },
{136, 380, 567, 119, 9,   577, 185, 574, 424, 81,  17,  371, 637, 581, 82,  153, 104, 557, 53,  91,  83,  289, 484, 477, 172, 100, 13,  474, 411, 254, 253},
{191, 566, 361, 386, 590, 374, 257, 112, 496, 51,  599, 134, 127, 96,  52,  137, 47,  488, 482, 642, 511, 304, 600, 325, 40,  457, 438, 74,  421, 92,  594},
{37,  51,  569, 624, 23,  192, 514, 566, 513, 529, 462, 619, 497, 596, 305, 82,  398, 228, 528, 425, 568, 587, 631, 282, 549, 543, 321, 15,  555, 106, 627},
{155, 229, 462, 222, 38,  602, 109, 370, 504, 122, 402, 176, 179, 644, 9,   73,  78,  192, 15,  342, 130, 320, 25,  570, 437, 313, 262, 289, 508, 81,  657},
{417, 255, 1,   417, 255, 1,   417, 255, 1,   417, 255, 1,   417, 255, 1,   417, 255, 1,   417, 255, 1,   417, 255, 1,   417, 255, 1,   417, 255, 1,   417},
{87,  122, 459, 667, 484, 134, 149, 100, 576, 14,  541, 354, 333, 537, 6,   522, 55,  46,  617, 196, 127, 217, 600, 71,  84,  538, 93,  644, 514, 36,  424},
{341, 171, 256, 555, 64,  651, 16,  675, 4,   681, 1,   341, 171, 256, 555, 64,  651, 16,  675, 4,   681, 1,   341, 171, 256, 555, 64,  651, 16,  675, 4  },
{134, 681, 42,  100, 271, 382, 54,  326, 151, 195, 563, 123, 589, 152, 329, 553, 165, 689, 423, 20,  607, 491, 149, 618, 583, 39,  389, 301, 256, 445, 204},
{582, 141, 45,  253, 36,  623, 169, 218, 696, 595, 697, 476, 137, 521, 390, 557, 312, 25,  530, 20,  424, 16,  199, 153, 19,  543, 576, 154, 601, 684, 621},
{567, 312, 363, 211, 525, 604, 21,  563, 171, 533, 177, 390, 631, 441, 479, 46,  558, 172, 391, 489, 44,  133, 257, 374, 67,  412, 343, 215, 666, 434, 55 },
{664, 149, 433, 631, 526, 549, 3,   554, 447, 580, 455, 140, 209, 9,   224, 622, 302, 646, 420, 627, 27,  672, 428, 187, 500, 541, 443, 81,  578, 565, 561},
{331, 511, 477, 128, 202, 705, 715, 390, 411, 92,  645, 484, 264, 144, 409, 157, 350, 257, 8,   467, 453, 181, 297, 162, 551, 631, 212, 380, 9,   71,  237},
{708, 625, 501, 669, 134, 315, 188, 431, 220, 364, 429, 270, 580, 160, 398, 312, 263, 22,  183, 556, 27,  58,  16,  333, 471, 686, 442, 678, 642, 76,  299},
{539, 94,  414, 707, 488, 687, 54,  285, 642, 186, 489, 487, 148, 699, 610, 674, 437, 541, 433, 602, 57,  424, 185, 689, 393, 473, 731, 122, 726, 383, 256},
{71,  583, 528, 338, 222, 159, 144, 565, 736, 246, 377, 19,  606, 675, 373, 478, 503, 49,  507, 333, 610, 216, 476, 361, 369, 194, 400, 166, 641, 188, 717},
{549, 250, 568, 167, 61,  445, 230, 102, 424, 717, 109, 512, 214, 330, 179, 641, 441, 287, 604, 405, 49,  616, 234, 45,  673, 736, 26,  5,   492, 499, 587},
{620, 601, 176, 112, 553, 696, 30,  432, 619, 738, 332, 693, 441, 143, 91,  402, 187, 119, 351, 361, 505, 459, 705, 311, 542, 689, 232, 10,  144, 711, 246},
{490, 385, 683, 591, 410, 757, 323, 743, 312, 680, 643, 16,  230, 72,  274, 324, 472, 697, 602, 473, 426, 226, 395, 256, 636, 391, 579, 618, 703, 498, 500},
{19,  361, 707, 360, 688, 768, 750, 408, 62,  409, 81,  1,   19,  361, 707, 360, 688, 768, 750, 408, 62,  409, 81,  1,   19,  361, 707, 360, 688, 768, 750},
{733, 54,  159, 597, 83,  545, 617, 56,  79,  705, 401, 193, 10,  373, 540, 44,  559, 57,  39,  759, 560, 17,  93,  145, 384, 100, 638, 762, 440, 179, 570},
{579, 766, 433, 441, 351, 183, 499, 92,  539, 429, 486, 435, 25,  309, 262, 594, 7,   118, 640, 670, 726, 96,  494, 345, 644, 625, 642, 254, 684, 175, 589},
{447, 559, 412, 57,  772, 780, 371, 61,  169, 625, 425, 289, 69,  557, 315, 533, 745, 666, 421, 95,  224, 503, 87,  633, 16,  776, 177, 216, 115, 397, 525},
{49,  783, 344, 676, 764, 222, 361, 700, 322, 407, 527, 744, 51,  72,  292, 555, 498, 132, 805, 613, 104, 242, 532, 180, 730, 174, 436, 330, 799, 319, 260},
{506, 571, 210, 19,  693, 306, 746, 361, 191, 137, 387, 371, 385, 170, 54,  561, 16,  797, 215, 116, 304, 545, 30,  582, 99,  623, 570, 515, 259, 483, 287},
{211, 187, 49,  487, 132, 759, 54,  721, 246, 183, 26,  560, 757, 453, 347, 148, 30,  583, 684, 649, 653, 676, 603, 799, 284, 812, 564, 780, 380, 543, 454},
{240, 813, 69,  100, 133, 646, 316, 124, 132, 406, 326, 55,  32,  273, 503, 562, 731, 141, 97,  236, 676, 109, 647, 556, 114, 201, 506, 459, 701, 348, 397},
{686, 33,  309, 262, 273, 376, 739, 3,   404, 99,  100, 786, 819, 301, 563, 9,   385, 297, 300, 704, 803, 76,  35,  27,  328, 64,  73,  458, 755, 228, 105},
{367, 391, 80,  345, 607, 597, 243, 478, 507, 373, 106, 768, 825, 190, 94,  509, 278, 59,  99,  686, 575, 459, 166, 405, 244, 16,  69,  453, 451, 546, 593},
{446, 73,  676, 295, 686, 560, 577, 608, 171, 756, 737, 653, 105, 685, 114, 504, 771, 715, 70,  177, 76,  336, 514, 197, 606, 118, 610, 224, 63,  411, 404},
{553, 435, 9,   712, 503, 81,  437, 262, 729, 521, 652, 590, 424, 750, 192, 404, 779, 22,  224, 187, 198, 310, 830, 76,  231, 646, 684, 373, 696, 185, 798},
{386, 735, 43,  315, 753, 135, 690, 670, 663, 532, 529, 228, 594, 465, 377, 689, 284, 785, 489, 214, 332, 459, 632, 564, 26,  609, 256, 261, 477, 724, 82 },
{797, 408, 474, 677, 117, 477, 491, 482, 181, 804, 833, 753, 559, 561, 437, 394, 483, 119, 353, 448, 571, 676, 179, 69,  17,  664, 64,  327, 342, 271, 378},
{115, 280, 269, 730, 239, 732, 469, 429, 144, 163, 622, 764, 697, 759, 122, 222, 503, 24,  171, 679, 415, 260, 558, 308, 37,  803, 4,   460, 257, 213, 331},
{116, 301, 713, 270, 625, 586, 447, 109, 366, 360, 541, 489, 596, 730, 488, 480, 429, 652, 210, 681, 66,  640, 572, 577, 280, 31,  88,  561, 178, 477, 81 },
{672, 512, 474, 487, 413, 21,  16,  180, 263, 536, 744, 441, 336, 256, 237, 684, 647, 451, 8,   90,  572, 268, 372, 661, 168, 128, 559, 342, 764, 666, 4  },
{550, 514, 140, 179, 437, 174, 336, 253, 519, 241, 100, 254, 186, 755, 240, 433, 623, 46,  576, 686, 259, 287, 676, 57,  445, 159, 33,  490, 185, 205, 609},
{647, 832, 782, 364, 453, 381, 808, 333, 797, 312, 515, 580, 59,  32,  303, 14,  188, 117, 304, 661, 133, 12,  668, 227, 514, 820, 114, 137, 826, 448, 694},
{311, 579, 483, 558, 301, 190, 135, 263, 163, 808, 49,  727, 254, 85,  132, 237, 240, 266, 189, 731, 591, 587, 250, 655, 537, 119, 729, 876, 336, 191, 446},
{403, 251, 32,  142, 744, 113, 900, 122, 883, 559, 260, 15,  579, 121, 480, 308, 228, 784, 746, 8,   491, 186, 256, 225, 486, 904, 823, 65,  687, 828, 258},
{578, 487, 272, 67,  128, 464, 763, 813, 305, 761, 576, 250, 217, 442, 913, 208, 754, 206, 517, 151, 892, 17,  636, 8,   29,  220, 338, 536, 105, 36,  590},
{561, 719, 173, 437, 830, 201, 352, 524, 400, 511, 539, 454, 148, 347, 506, 521, 575, 212, 20,  72,  445, 673, 379, 807, 304, 537, 261, 568, 1,   561, 719},
{105, 718, 430, 174, 467, 311, 797, 292, 676, 705, 2,   210, 499, 860, 348, 934, 622, 657, 584, 415, 473, 4,   420, 61,  783, 696, 931, 307, 377, 231, 830},
{518, 139, 486, 501, 743, 5,   708, 695, 548, 623, 892, 25,  717, 652, 858, 292, 696, 125, 762, 437, 526, 519, 657, 625, 46,  303, 748, 713, 462, 302, 230},
{316, 421, 456, 152, 682, 543, 181, 376, 441, 147, 49,  332, 742, 563, 819, 273, 91,  346, 431, 775, 574, 507, 169, 372, 124, 357, 119, 671, 855, 285, 95 },
{238, 417, 134, 443, 604, 802, 276, 884, 732, 770, 284, 882, 256, 889, 16,  949, 1,   238, 417, 134, 443, 604, 802, 276, 884, 732, 770, 284, 882, 256, 889},
{50,  566, 257, 279, 412, 293, 145, 481, 842, 519, 808, 753, 904, 718, 121, 248, 796, 153, 881, 535, 641, 139, 181, 347, 911, 101, 215, 113, 815, 136, 31 },
{285, 632, 485, 343, 655, 243, 314, 158, 364, 814, 892, 789, 564, 525, 91,  689, 223, 440, 141, 374, 751, 415, 784, 110, 278, 579, 916, 832, 196, 513, 555},
{67,  581, 824, 496, 14,  938, 318, 789, 105, 196, 431, 544, 299, 493, 790, 172, 777, 278, 63,  313, 454, 131, 961, 882, 474, 494, 857, 753, 624, 774, 77 },
{444, 536, 98,  260, 429, 757, 905, 756, 461, 220, 363, 943, 917, 186, 12,  413, 534, 193, 171, 233, 237, 47,  225, 617, 674, 424, 503, 191, 266, 144, 41 },
{53,  827, 227, 139, 430, 988, 832, 492, 310, 574, 692, 9,   477, 506, 61,  260, 897, 964, 551, 464, 808, 211, 282, 81,  329, 590, 549, 358, 145, 748, 4  },
{966, 961, 119, 299, 701, 203, 686, 668, 229, 877, 729, 332, 675, 12,  625, 565, 431, 597, 436, 442, 256, 40,  754, 554, 772, 993, 124, 144, 521, 798, 187},
{383, 384, 767, 142, 909, 42,  951, 993, 935, 919, 845, 755, 591, 337, 928, 256, 175, 431, 606, 28,  634, 662, 287, 949, 227, 167, 394, 561, 955, 507, 453}};

#endif
