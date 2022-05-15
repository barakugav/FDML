syms d1 d2 k m2 real

p1 = -(d1^4 + d2^4*m2^4 + 4*k^2*m2^2 - 2*d1^2*d2^2*m2^2)/(m2^2 + 1)^2						% x^4
p2 = -(4*d1^4*m2 - 8*k^2*m2 - 4*d2^4*m2^3 + 8*k^2*m2^3 + 4*d1^2*d2^2*m2 - 4*d1^2*d2^2*m2^3)/(m2^2 + 1)^2	% x^3*y
p3 = -(4*k^2 - 2*d1^2*d2^2 + 6*d1^4*m2^2 + 6*d2^4*m2^2 - 16*k^2*m2^2 + 4*k^2*m2^4 + 8*d1^2*d2^2*m2^2 - 2*d1^2*d2^2*m2^4)/(m2^2 + 1)^2	% x^2*y^2
p4 = (2*d1^2*k^2 + 2*d1^2*k^2*m2^2 + 2*d2^2*k^2*m2^2 + 2*d2^2*k^2*m2^4)/(m2^2 + 1)^2				% x^2
p5 = (4*d2^4*m2 - 8*k^2*m2 - 4*d1^4*m2^3 + 8*k^2*m2^3 + 4*d1^2*d2^2*m2 - 4*d1^2*d2^2*m2^3)/(m2^2 + 1)^2		% x*y^3
p6 = (4*d1^2*k^2*m2 - 4*d2^2*k^2*m2 + 4*d1^2*k^2*m2^3 - 4*d2^2*k^2*m2^3)/(m2^2 + 1)^2				% x*y
p7 = -(d2^4 + d1^4*m2^4 + 4*k^2*m2^2 - 2*d1^2*d2^2*m2^2)/(m2^2 + 1)^2						% y^4
p8 = (2*d2^2*k^2 + 2*d1^2*k^2*m2^2 + 2*d2^2*k^2*m2^2 + 2*d1^2*k^2*m2^4)/(m2^2 + 1)^2 				% y^2
p9 = -(k^4 + 2*k^4*m2^2 + k^4*m2^4)/(m2^2 + 1)^2			    					% free

[p1, p2, p3, p4, p5, p6, p7, p8, p9] = negate_terms(p1, p2, p3, p4, p5, p6, p7, p8, p9)

fid_c_o = fopen('sol_c_o.txt', 'wt')

fprintf(fid_c_o, '%s\n', p1)
fprintf(fid_c_o, '%s\n', p2)
fprintf(fid_c_o, '%s\n', p3)
fprintf(fid_c_o, '%s\n', p4)
fprintf(fid_c_o, '%s\n', p5)
fprintf(fid_c_o, '%s\n', p6)
fprintf(fid_c_o, '%s\n', p7)
fprintf(fid_c_o, '%s\n', p8)
fprintf(fid_c_o, '%s\n', p9)
fprintf(fid_c_o, '\n')

syms A1 B1 C1 D1 A2 B2 C2 D2 real

% eq10 = A1 == d1^2*m2 - d2^2*m1 + 2*sqrt(d1^2*d2^2 - k^2)
% eq11 = B1 == d2^2*m2 - d1^2*m1 - 2*sqrt(d1^2*d2^2 - k^2)
% eq12 = C1 == 2 * (d2^2 - d1^2) + 2 * (m1 + m2) * sqrt(d1^2*d2^2 - k^2)
% eq13 = D1 == k^2*(m2 - m1)
% eq14 = A2 == d1^2*m2 - d2^2*m1 - 2*sqrt(d1^2*d2^2 - k^2)
% eq15 = B2 == d2^2*m2 - d1^2*m1 + 2*sqrt(d1^2*d2^2 - k^2)
% eq16 = C2 == 2 * (d2^2 - d1^2) - 2 * (m1 + m2) * sqrt(d1^2*d2^2 - k^2)
% eq17 = D2 == k^2*(m1 - m2)

% m1 = (-1/m2)
% eq10 = A1 == d1^2*m2 - d2^2*(-1/m2) + 2*sqrt(d1^2*d2^2 - k^2)
% eq11 = B1 == d2^2*m2 - d1^2*(-1/m2) - 2*sqrt(d1^2*d2^2 - k^2)
% eq12 = C1 == 2 * (d2^2 - d1^2) + 2 * ((-1/m2) + m2) * sqrt(d1^2*d2^2 - k^2)
% eq13 = D1 == k^2*(m2 - (-1/m2))
eq13 = D1 == k^2*(m2 - (-1/m2))
% eq14 = A2 == d1^2*m2 - d2^2*(-1/m2) - 2*sqrt(d1^2*d2^2 - k^2)
% eq15 = B2 == d2^2*m2 - d1^2*(-1/m2) + 2*sqrt(d1^2*d2^2 - k^2)
% eq16 = C2 == 2 * (d2^2 - d1^2) - 2 * ((-1/m2) + m2) * sqrt(d1^2*d2^2 - k^2)
% eq17 = D2 == k^2*((-1/m2) - m2)
eq17 = D2 == k^2*((-1/m2) - m2)

sol_c_o = compute_coeffs(p1, p2, p3, p4, p5, p6, p7, p8, p9, eq17)
print_coeffs(fid_c_o, sol_c_o)
fclose(fid_c_o)
