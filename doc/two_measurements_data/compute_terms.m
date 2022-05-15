function [p1, p2, p3, p4, p5, p6, p7, p8, p9] = compute_terms(sol)
  syms d1 d2 k m2 x y real
  [cxy, terms] = coeffs(sol, [x y], 'All')
  [cols, rows] = size(cxy)
  assert(isequal(cols, 5))
  assert(isequal(rows, 5))

  p1 = cxy(1, 5) % x^4
  p2 = cxy(2, 4) % x^3*y
  p3 = cxy(3, 3) % x^2*y^2
  p4 = cxy(3, 5) % x^2
  p5 = cxy(4, 2) % x*y^3
  p6 = cxy(4, 4) % x*y
  p7 = cxy(5, 1) % y^4
  p8 = cxy(5, 3) % y^2
  p9 = cxy(5, 5) % free
end
