#include "Commons.hlsli"

// 사용할 이미지를 GPU 메모리에서 Texture2D 형식으로 받아온다.
Texture2D g_texture0 : register(t0); // t는 texture
// Texture Image 안의 원하는 위치에서 색깔값을 가져오는 작업을 Sampling이라 한다.
SamplerState g_sampler0 : register(s0); // s는 sampler

cbuffer MeshPSConstData : register(b0)
{
    float3 camWorld;            // 시점의 월드 좌표계 위치
    bool useTexture;    
    Material material;          // 재질
    Light lights[MAX_LIGHTS]; // 조명
    RimLight rimLights;
};

// object의 테두리에 빛이 나는 역광 조명 효과
float3 RimLighting(float3 color, float3 toCam, float3 normal)
{
    // 모델의 중심에 가까울수록 dot product의 값은 1.0에 가까워진다. (안으로 갈수록 rim값은 0.0에 가까워진다.)
    float rim = 1.0f - dot(toCam, normal);
    
    // Smoothstep
    // https://thebookofshaders.com/glossary/?search=smoothstep
    if (rimLights.useSmoothStep)
        rim = smoothstep(0.0f, 1.0f, rim); // 내장함수 사용
    
    rim = pow(abs(rim), rimLights.rimPower);

    return color + rim * rimLights.rimColor * rimLights.rimStrength;
}

float4 psMain(PSInput input) : SV_TARGET
{
    float3 toCam = normalize(camWorld - input.posWorld);
    float3 color = input.color;
    
    // https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-for
    // https://forum.unity.com/threads/what-are-unroll-and-loop-when-to-use-them.1283096/
    int i = 0;
    // 각 조명의 index를 생각해서 for loop를 돌려야된다.
    [unroll] // warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
    for (i = 0; i < NUM_DIR_LIGHTS; ++i)
        color += DirectionalLight(lights[i], input.normalWorld, toCam, material);
    [unroll] 
    for (i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; ++i)
        color += PointLight(lights[i], input.posWorld, input.normalWorld, toCam, material);
    [unroll] 
    for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
        color += SpotLight(lights[i], input.posWorld, input.normalWorld, toCam, material);
     
    // rim 조명 효과
    // color = RimLighting(color, toCam, input.normalWorld);
    
    return useTexture ? float4(color, 1.0) * g_texture0.Sample(g_sampler0, input.texcoord) : float4(color, 1.0f);
}