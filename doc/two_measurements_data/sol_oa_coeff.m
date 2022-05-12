syms d1 d2 k m2
syms A1 B1 C1 D1 A2 B2 C2 D2
eq1 = A1*A2 == d1^4					% x^4
eq2 = B1*B2 == d2^4					% y^4
eq3 = D1*D2 == k^4					%

eq4 = A1*D2 + A2*D1 == -2*d1^2*k^2			% x^2
eq5 = B1*D2 + B2*D1 == -2*d2^2*k^2			% y^2

eq6 = C1*D2+C2*D1 == 0					% x*y

eq7 = A1*B2 + A2*B1 + C1*C2 == 4*k^2 - 2*d1^2*d2^2	% x^2*y^2

sol_c = solve([eq1, eq2, eq3, eq4, eq5, eq6, eq7],[A1, B1, C1, D1, A2, B2, C2, D2])
fid = fopen('sol_c_oa.txt', 'wt')
fprintf(fid, '%s, %s, %s, %s\n', char(sol_c.A1), char(sol_c.B1), char(sol_c.C1), char(sol_c.D1))
fprintf(fid, '%s, %s, %s, %s\n', char(sol_c.A2), char(sol_c.B2), char(sol_c.C2), char(sol_c.D2))
fclose(fid)
