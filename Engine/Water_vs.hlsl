// Light vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    float time;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 position3D : TEXCOORD2;
    float time : TIME;
};




int FastFloor(double x) 
{
    return x > 0 ? (int)x : (int)x - 1;
}
double Dot(int g[3], double x, double y, double z) 
{
    return g[0] * x + g[1] * y + g[2] * z;
}
double Mix(double a, double b, double t) 
{
    return (1 - t) * a + t * b;
}
double Fade(double t) 
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double Noise(double x, double y, double z) {

    int grad3[12][3] = { {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
                                 {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
                                 {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1} };;
    int p[256] = { 151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };
    int perm[512];


    for (int i = 0; i < 512; i++)
    {
        perm[i] = p[i & 255];
    }


    int X = FastFloor(x);
    int Y = FastFloor(y);
    int Z = FastFloor(z);

    // Get relative xyz coordinates of point within that cell
    x = x - X;
    y = y - Y;
    z = z - Z;

    // Wrap the integer cells at 255 (smaller integer period can be introduced here)
    X = X & 255;
    Y = Y & 255;
    Z = Z & 255;
    int gi000 = perm[X + perm[Y + perm[Z]]] % 12;
    int gi001 = perm[X + perm[Y + perm[Z + 1]]] % 12;
    int gi010 = perm[X + perm[Y + 1 + perm[Z]]] % 12;
    int gi011 = perm[X + perm[Y + 1 + perm[Z + 1]]] % 12;
    int gi100 = perm[X + 1 + perm[Y + perm[Z]]] % 12;
    int gi101 = perm[X + 1 + perm[Y + perm[Z + 1]]] % 12;
    int gi110 = perm[X + 1 + perm[Y + 1 + perm[Z]]] % 12;
    int gi111 = perm[X + 1 + perm[Y + 1 + perm[Z + 1]]] % 12;
 
   // Calculate noise contributions from each of the eight corners
    double n000 = Dot(grad3[gi000], x, y, z);
    double n100 = Dot(grad3[gi100], x - 1, y, z);
    double n010 = Dot(grad3[gi010], x, y - 1, z);
    double n110 = Dot(grad3[gi110], x - 1, y - 1, z);
    double n001 = Dot(grad3[gi001], x, y, z - 1);
    double n101 = Dot(grad3[gi101], x - 1, y, z - 1);
    double n011 = Dot(grad3[gi011], x, y - 1, z - 1);
    double n111 = Dot(grad3[gi111], x - 1, y - 1, z - 1);
    // Compute the fade curve value for each of x, y, z
    double u = Fade(x);
    double v = Fade(y);
    double w = Fade(z);
    // Interpolate along x the contributions from each of the corners
    double nx00 = Mix(n000, n100, u);
    double nx01 = Mix(n001, n101, u);
    double nx10 = Mix(n010, n110, u);
    double nx11 = Mix(n011, n111, u);
    // Interpolate the four results along y
    double nxy0 = Mix(nx00, nx10, v);
    double nxy1 = Mix(nx01, nx11, v);
    // Interpolate the two last results along z
    double nxyz = Mix(nxy0, nxy1, w);

    return nxyz;
}

OutputType main(InputType input)
{


    OutputType output;
    input.position.w = 1.0f;
    //output.position.y = sin(output.position.x * time / 10);


    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);

    /*output.position.x += time;
    output.position.z += time;*/
    float noise = Noise((output.position.z + time / 1000) * 0.1, (output.position.x + time / 1000) * 0.1, 1) * 2;

    output.position.y = sin((output.position.x + output.position.z) - time) + noise;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the texture coordinates for the pixel shader.
    output.tex = input.tex;



    // Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3)worldMatrix);

    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    // world position of vertex (for point light)
    output.position3D = (float3)mul(input.position, worldMatrix);


    //Other algorithm for water

    /*float speed = 0.4f;
    float freq = 1;
    float amp = 0.3f;
    float t = time * speed;

    float waveLength = sin(t + output.position.x * freq) * amp + sin(t * 2 + output.position.z * freq * 2) * amp;
    output.position.y += waveLength;

    output.normal = normalize(float3(output.normal.x + waveLength, output.normal.y, output.normal.z + waveLength));*/

    /*for (int j = 0; j < m_terrainHeight; j++)
    {
        for (int i = 0; i < m_terrainWidth; i++)
        {

            index = (m_terrainHeight * j) + i;
            double noise = 5 * perlinNoise.Noise((i + m_offset) * m_scale, (j + m_offset) * m_scale, 1);
            m_heightMap[index].x = (float)i;
            m_heightMap[index].y = noise;
            m_heightMap[index].z = (float)j;


        }
    }*/




    return output;
}