#version 330 core

in vec2 fragCoord;
out vec4 color;

uniform vec3 location;
uniform vec2 screeResolution;
uniform mat3 viewToWorld;
uniform sampler2D texPosition;
uniform sampler2D texNode;
uniform int bvhWidth;
uniform int texPosWidth;


//------------------- STRUCT AND LOADER BEGIN -----------------------
struct Triangle
{
    // Position
    vec3 pos1;
    vec3 pos2;
    vec3 pos3;
};
struct Node
{
	int childIsTriangle;
    int leftChild;
    int rightChild;
    vec3 aabbMin;
    vec3 aabbMax;
};

struct Ray
{
    vec3 origin;
    vec3 direction;
    float tStart;
    float tEnd;
};

struct Hit
{
    vec3 normal;
    vec3 position;
    vec2 uv;
    bool isHit;
};


vec2 get2DIndex(int index, int width)
{
	int x = index % width;
    int y = index / width;
    return vec2(x / float(width), y / float(width));
}

Node getNode(int index)
{
	index = index * 3;

	vec3 integerData = texture(texNode, get2DIndex(index, bvhWidth)).rgb;
	vec3 aabbMin = texture(texNode, get2DIndex(index + 1, bvhWidth)).rgb;
	vec3 aabbMax = texture(texNode, get2DIndex(index + 2, bvhWidth)).rgb;

	Node node;
	node.childIsTriangle = int(integerData.x);
	node.leftChild = int(integerData.y);
	node.rightChild = int(integerData.z);
	node.aabbMin = aabbMin;
	node.aabbMax = aabbMax;
	return node;
}

Triangle getTriangle(int index)
{
	index = index * 3;
	Triangle triangle;
	triangle.pos1 = texture(texPosition, get2DIndex(index, texPosWidth)).rgb;
	triangle.pos2 = texture(texPosition, get2DIndex(index + 1, texPosWidth)).rgb;
	triangle.pos3 = texture(texPosition, get2DIndex(index + 2, texPosWidth)).rgb;
	return triangle;
}
//------------------- STRUCT AND LOADER END -----------------------

//------------------- STACK BEGIN -----------------------
int countTI = 0;
int _stack[15];
int _index = -1;

void stackClear()
{
    _index = -1;
}

int stackSize()
{
    return _index + 1;
}

void stackPush(in int node)
{
    if(_index > 14	)
        discard;
    _stack[++_index] = node;
 
}

int stackPop()
{
    return _stack[_index--];
}
//------------------- STACK END -----------------------

bool slabs(in Ray ray, in vec3 minB, in vec3 maxB, inout float localMin) {

    if(all(greaterThan(ray.origin, minB)) && all(lessThan(ray.origin, maxB)))
        return true;

    vec3 t0 = (minB - ray.origin)/ray.direction;
    vec3 t1 = (maxB - ray.origin)/ray.direction;
    vec3 tmin = min(t0, t1), tmax = max(t0, t1);
    float tminf = max(max(tmin.x, tmin.y), tmin.z);
    float tmaxf = min(min(tmax.x, tmax.y), tmax.z);

    if (tminf > tmaxf)
        return false;

    localMin = tminf;
    return tminf < ray.tEnd && tminf > ray.tStart;
}

bool isect_tri(inout Ray ray, in Triangle tri, inout Hit hit ) {
	vec3 e1 = tri.pos2 - tri.pos1;
	vec3 e2 = tri.pos3 - tri.pos1;
	vec3 P = cross(ray.direction, e2);
	float det = dot(e1, P);
	if (abs(det) < 1e-4)
        return false;

	float inv_det = 1. / det;
	vec3 T = (ray.origin - tri.pos1);
	float u = dot(T, P) * inv_det;
	if (u < 0.0 || u > 1.0)
        return false;

	vec3 Q = cross(T, e1);
	float v = dot(ray.direction, Q) * inv_det;
	if (v < 0.0 || (v+u) > 1.0)
        return false;

	float tt = dot(e2, Q) * inv_det;

    if(ray.tEnd > tt && ray.tStart < tt )
    {
        vec3 c = vec3(u, v, 1.0 - u - v);
        countTI++;
        hit.normal = normalize(cross(e1, e2));//(tri.norm1 * c.z + tri.norm2 * c.x + tri.norm3 * c.y);
        //chit.uv = tri.uv1 * c.z + tri.uv2 * c.x + tri.uv3 * c.y;
        hit.position = (ray.origin + ray.direction * tt);
        hit.isHit = true;
        ray.tEnd = tt;
        return true;
    }
    return false;
}

void traceCloseHitV2(inout Ray ray, inout Hit hit)
{
    stackClear();
    stackPush(0);
    hit.isHit = false;
    Node select;
    Triangle try;
    float tempt;

    while(stackSize() != 0)
    {
        select = getNode(stackPop());
        if(!slabs(ray, select.aabbMin, select.aabbMax, tempt))
            continue;
        
        if(select.childIsTriangle == 0)
        {
            float leftMinT = 0;
            float rightMinT= 0;
            Node right = getNode(select.rightChild);
            Node left = getNode(select.leftChild);
            bool rightI = slabs(ray, right.aabbMin, right.aabbMax,  rightMinT);
            bool leftI = slabs(ray, left.aabbMin, left.aabbMax,  leftMinT);

            if(rightI && leftI)
            {
                if (rightMinT < leftMinT)
                {
                    stackPush(select.leftChild);
                    stackPush(select.rightChild);
                }
                else
                {
                    stackPush(select.rightChild);
                    stackPush(select.leftChild);
                }
                continue;
            }
            if(rightI)
                stackPush(select.rightChild);
            else
                stackPush(select.leftChild);
            continue;
        }

        if ((select.childIsTriangle & 2) == 0)
            stackPush(select.rightChild);

        if ((select.childIsTriangle & 1) == 0)
            stackPush(select.leftChild);

        if((select.childIsTriangle & 2) > 0)
        {
            try = getTriangle(select.rightChild);
            isect_tri(ray, try, hit);
        }

        if((select.childIsTriangle & 1) > 0)
        {
            try = getTriangle(select.leftChild);
            isect_tri(ray, try, hit);
        }
    }
}

mat3 rotationMatrix(vec3 axis, float angle)
{
   axis = normalize(axis);
   float s = sin(angle);
   float c = cos(angle);
   float oc = 1.0 - c;

   return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
               oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
               oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

void traceCloseFor(inout Ray ray, inout Hit hit)
{
	hit.isHit = false;

	Node nod = getNode(2230);
	float temp = 0;

	//slabs(ray, vec3(-2.3, -5.0, -9.16), vec3(2.3, 5.0, 9), temp);
	slabs(ray, nod.aabbMin, nod.aabbMax, temp);
	hit.normal = vec3(0, 0, 0);
	// if(nod.childIsTriangle == 0)
	// 	chit.normal = vec3(1, 0, 0);
	if(temp > 0)
		hit.normal = vec3(1, 0, 0);

	// for(int i = 0; i< 15216; i++)
	// {
	// 	Triangle tri = getTriangle(i);
	// 	isect_tri(ray, tri, chit);
	// }
}

void main() {
    vec3 viewDir = normalize(vec3((gl_FragCoord.xy - screeResolution.xy*0.5) / screeResolution.y, 1.0));
    vec3 worldDir = viewToWorld * viewDir;

    Ray ray;
    ray.direction = worldDir;
    ray.origin = location;
    ray.tStart = 0.0001;
    ray.tEnd = 10000;

    Hit hit;
    //traceCloseFor(ray, hit);
    traceCloseHitV2(ray, hit);
    //color = vec4(fragCoord,0.0,1.0);
    color = vec4(0.5+hit.normal*0.5, 1.0);
}
