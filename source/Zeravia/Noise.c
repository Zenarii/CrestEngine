//Simplex Permutations
global const i32 SNPs[] = {151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,
    130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,
    38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,
    170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,
    253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,34,242,
    193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,
    130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,
    38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,
    170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,
    253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,34,242,
    193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

internal i32
NoiseFastFloor(r32 x) {
    return (x > 0) ? x : (x-1);
}


/*
    3D Noise
*/
global v3 SimplexNoise3DGradients[] = {{1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
                                       {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
                                       {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}};


internal r32
SimplexNoise3D(v3 Point) {
    //Note(Zen): Noise Contributions from the corners
    r32 SkewFactor = 1.f/3.f; //[sqrt(n + 1) - 1]/3
    r32 Stretch = (Point.x + Point.y + Point.z) * SkewFactor;

    i32 i = NoiseFastFloor(Point.x + Stretch);
    i32 j = NoiseFastFloor(Point.y + Stretch);
    i32 k = NoiseFastFloor(Point.z + Stretch);

    r32 UnskewFactor = 1.f/6.f;
    r32 t = (i + j + k) * UnskewFactor;
    //unskewed origin
    r32 X0 = i - t;
    r32 Y0 = j - t;
    r32 Z0 = k - t;
    //distances within cell
    r32 x0 = Point.x - X0;
    r32 y0 = Point.y - Y0;
    r32 z0 = Point.z - Z0;

    i32 i1, j1, k1;
    i32 i2, j2, k2;

    if(x0 >= y0) {
        if(y0 >= z0) {
            i1 = 1;
            j1 = 0;
            i2 = 1;
            k1 = 1;
            j2 = 1;
            k2 = 0;
        }
        else if(x0 >= z0) {
            i1 = 1;
            j1 = 0;
            k1 = 0;
            i2 = 1;
            j2 = 0;
            k2 = 1;
        }
        else {
             i1 = 0;
             j1 = 0;
             k1 = 1;
             i2 = 1;
             j2 = 0;
             k2 = 1;
        }
    }
    else {
        if(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; }
        else if(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; }
        else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; }
    }

    r32 x1 = x0 - i1 + UnskewFactor;
    r32 y1 = y0 - j1 + UnskewFactor;
    r32 z1 = z0 - k1 + UnskewFactor;

    r32 x2 = x0 - i2  + 2.f * UnskewFactor;
    r32 y2 = y0 - j2 + 2.f * UnskewFactor;
    r32 z2 = z0 - k2 + 2.f * UnskewFactor;

    r32 x3 = x0 - 1.f + 3.f * UnskewFactor;
    r32 y3 = y0 - 1.f + 3.f * UnskewFactor;
    r32 z3 = z0 - 1.f + 3.f * UnskewFactor;

    //Get hashed gradient of corners
    i32 ii = i & 255;
    i32 jj = j & 255;
    i32 kk = k & 255;
    i32 gi0 = SNPs[ii+SNPs[jj+SNPs[kk]]] % 12;
    i32 gi1 = SNPs[ii+i1+SNPs[jj+j1+SNPs[kk+k1]]] % 12;
    i32 gi2 = SNPs[ii+i2+SNPs[jj+j2+SNPs[kk+k2]]] % 12;
    i32 gi3 = SNPs[ii+1+SNPs[jj+1+SNPs[kk+1]]] % 12;

    r32 n0, n1, n2, n3;
    //Get Contributions form each corner
    r32 t0 = 0.5f - x0*x0 - y0*y0 - z0*z0;
    if(t0 < 0.f) n0 = 0.f;
    else {
        t0 = t0 * t0;
        n0 = t0 * t0 * CrestV3Dot(SimplexNoise3DGradients[gi0], v3(x0, y0, z0));
    }

    r32 t1 = 0.5f - x1*x1 - y1*y1 - z1*z1;
    if(t1 < 0.f) n1 = 0.f;
    else {
        t1 = t1 * t1;
        n1 = t1 * t1 * CrestV3Dot(SimplexNoise3DGradients[gi1], v3(x1, y1, z1));
    }

    r32 t2 = 0.5f - x2*x2 - y2*y2 - z2*z2;
    if(t2 < 0.f) n2 = 0.f;
    else {
        t2 = t2 * t2;
        n2 = t2 * t2 * CrestV3Dot(SimplexNoise3DGradients[gi2], v3(x2, y2, z2));
    }

    r32 t3 = 0.5f - x3 * x3 - y3*y3 - z3 * z3;
    if(t3 < 0.f) n3 = 0.f;
    else {
        t3 = t3 * t3;
        n3 = t3 * t3 * CrestV3Dot(SimplexNoise3DGradients[gi3], v3(x3, y3, z3));
    }

    r32 Result = 32.f * (n0 + n1 + n2 + n3);
    Assert(Result < 1.f);
    Assert(Result > -1.f);

    return Result;
}
#undef SimplexNoisePermutations

internal v3
Noise3DSample(v3 Input) {
    v3 Sample = v3(0.f, 0.f, 0.f);
    Sample.x = SimplexNoise3D(v3(Input.x + Input.x, Input.y + Input.x, Input.z + Input.x));
    Sample.y = SimplexNoise3D(v3(Input.x + Input.y, Input.y + Input.y, Input.z + Input.y));
    Sample.z = SimplexNoise3D(v3(Input.x + Input.z, Input.y + Input.z, Input.z + Input.z));
    return Sample;
}
