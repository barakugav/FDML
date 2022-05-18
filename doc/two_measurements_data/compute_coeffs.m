function [sol_coeffs] = compute_coeffs(p1, p2, p3, p4, p5, p6, p7, p8, p9, hint)
  syms A1 B1 C1 D1 A2 B2 C2 D2 real
  eq1 = A1*A2 == p1		    	% x^4
  eq2 = A1*C2 + A2*C1 == p2	    	% x^3*y
  eq3 = A1*B2 + A2*B1 + C1*C2 == p3	% x^2*y^2
  eq4 = A1*D2 + A2*D1 == p4	    	% x^2
  eq5 = B1*C2 + B2*C1 == p5		% x*y^3
  eq6 = C1*D2 + C2*D1 == p6           	% x*y
  eq7 = B1*B2 == p7                   	% y^4
  eq8 = B1*D2 + B2*D1 == p8           	% y^2
  eq9 = D1*D2 == p9                   	% free

  if nargin > 9
    sol_coeffs = solve([eq1, eq2, eq3, eq4, eq5, eq6, eq7, eq8, eq9, hint], [A1, B1, C1, D1, A2, B2, C2, D2], 'Real', true)
  else
    sol_coeffs = solve([eq1, eq2, eq3, eq4, eq5, eq6, eq7, eq8, eq9], [A1, B1, C1, D1, A2, B2, C2, D2], 'Real', true)
  end
end
