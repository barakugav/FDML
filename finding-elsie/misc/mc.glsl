
// BOUNDING_BOX_POSITION 0 0 0
// BOUNDING_BOX_SIZE 5 5 7

// Your glsl signed distance function:
float mapDistance(vec3 p) {
  const float infinity = 1000.0;
	const float pi = 3.14159265;
  const int num_v = 4;
  
  vec2 polygon[num_v];
  polygon[0] = vec2(-1,-1);
  polygon[1] = vec2(-1,1);
  polygon[2] = vec2(1,1);
  polygon[3] = vec2(1,-1);
  
  float theta = pi/2.0 - p.z;
  float cos_theta = cos(theta);
  float sin_theta = sin(theta);
  vec2 rotatedPolygon[num_v + 1];
  for (int i = 0; i < num_v; i++)
  {
    float x_, y_;
    x_ = polygon[i].x - p.x;
    y_ = polygon[i].y - p.y;
    rotatedPolygon[i].x = x_ * cos_theta - y_ * sin_theta;
    rotatedPolygon[i].y = x_ * sin_theta + y_ * cos_theta;
  }
  rotatedPolygon[num_v] = rotatedPolygon[0];
  
  float min_d = infinity;
  for (int i = 0; i < num_v; i++)
  {
    vec2 u = rotatedPolygon[i];
    vec2 v = rotatedPolygon[i + 1];
    
    if (
    	!(u.x <= 0.0 && 0.0 <= v.x) &&
      !(v.x <= 0.0 && 0.0 <= u.x)
    ) 
      continue;
    
    float y_ = u.y - u.x * (v.y - u.y) / (v.x - u.x);
    if (y_ < 0.0) continue;
    
    if (y_ < min_d)
      min_d = y_;
  }
  
  
  return min_d - .2;
}

