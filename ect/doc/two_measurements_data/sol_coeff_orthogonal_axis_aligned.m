syms d1 d2 k m2 real

p1 = -d1^4			    % x^4
p2 = 0                              % x^3*y
p3 = - 4*k^2 + 2*d1^2*d2^2	    % x^2*y^2
p4 = 2*d1^2*k^2			    % x^2
p5 = 0                              % x*y^3
p6 = 0				    % x*y
p7 = -d2^4			    % y^4
p8 = 2*d2^2*k^2			    % y^2
p9 = -k^4			    % free

[p1, p2, p3, p4, p5, p6, p7, p8, p9] = negate_terms(p1, p2, p3, p4, p5, p6, p7, p8, p9)

fid_c_oa = fopen('sol_c_oa.txt', 'wt')

eq1 = A1*A2 == p1		    % x^4
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

eq10 = A1 == d1^2
eq13 = D1 == -k^2
eq14 = A2 == d2^2
eq17 = D2 == -k^2

sol_c_o = compute_coeffs(p1, p2, p3, p4, p5, p6, p7, p8, p9, eq13)
print_coeffs(fid_c_o, sol_c_o)

fclose(fid_c_oa)
