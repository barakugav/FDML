syms d1 d2 k m2
syms A1 B1 C1 D1 A2 B2 C2 D2
eq1 = A1*A2 == d1^4*m2^4
eq2 = B1*B2 == 4*k^2 + d1^4 + d2^4 - 4*d1^2*k - 4*d2^2*k + 2*d1^2*d2^2 + 2*d2^4*m2^2 + d2^4*m2^4 + 4*k^2*m2^2 - 4*d2^2*k*m2^2 - 2*d1^2*d2^2*m2^2
eq3 = D1*D2 == k^4 + d1^4*d2^4 + 2*k^4*m2^2 + k^4*m2^4 - 2*d1^2*d2^2*k^2 - 2*d1^2*d2^2*k^2*m2^2

eq4 = A1*D2 + A2*D1 == 2*d1^2*k^2*m2^2 - 2*d1^2*k^2*m2^4 - 2*d1^4*d2^2*m2^2
eq5 = B1*D2 + B2*D1 == - 4*k^3 - 2*d1^2*d2^4 - 2*d1^4*d2^2 + 2*d1^2*k^2 + 2*d2^2*k^2 - 4*k^3*m2^2 + 4*d1^2*d2^2*k - 2*d1^2*d2^4*m2^2 - 2*d1^2*k^2*m2^2 - 2*d2^2*k^2*m2^4 + 8*d1^2*d2^2*k*m2^2

eq6 = C1*D2+C2*D1 == + 4*k^3*m2 + 4*k^3*m2^3 + 4*d1^4*d2^2*m2 - 4*d1^2*k^2*m2 + 4*d1^2*k^2*m2^3 - 8*d1^2*d2^2*k*m2^3 - 4*d1^2*d2^2*k*m2

eq7 = A1*B2 + A2*B1 + C1*C2 == + 6*d1^4*m2^2 + 4*k^2*m2^2 + 4*k^2*m2^4 - 12*d1^2*k*m2^2 + 2*d1^2*d2^2*m2^2 - 2*d1^2*d2^2*m2^4

sol_c = solve([eq1, eq2, eq3, eq4, eq5, eq6, eq7],[A1, B1, C1, D1, A2, B2, C2, D2])
fid = fopen('sol_c_a.txt', 'wt')
A1 = reduceRedundancies([sol_c.A1], [d1, d2, k, m2])
fprintf(fid, 'A1 = %s\n', char(A1))
B1 = reduceRedundancies([sol_c.B1], [d1, d2, k, m2])
fprintf(fid, 'B1 = %s\n', char(B1))
C1 = reduceRedundancies([sol_c.C1], [d1, d2, k, m2])
fprintf(fid, 'C1 = %s\n', char(C1))
D1 = reduceRedundancies([sol_c.D1], [d1, d2, k, m2])
fprintf(fid, 'D1 = %s\n', char(D1))

fprintf(fid, '\n')
A2 = reduceRedundancies([sol_c.A2], [d1, d2, k, m2])
fprintf(fid, 'A2 = %s\n', char(A2))
B2 = reduceRedundancies([sol_c.B2], [d1, d2, k, m2])
fprintf(fid, 'B2 = %s\n', char(B2))
C2 = reduceRedundancies([sol_c.C2], [d1, d2, k, m2])
fprintf(fid, 'C2 = %s\n', char(C2))
D2 = reduceRedundancies([sol_c.D2], [d1, d2, k, m2])
fprintf(fid, 'D2 = %s\n', char(D2))
fclose(fid)
