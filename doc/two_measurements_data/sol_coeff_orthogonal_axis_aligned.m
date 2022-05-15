syms d1 d2 k m2 real
syms A1 B1 C1 D1 A2 B2 C2 D2
p1 = -d1^4			    % x^4
p2 = 0                              % x^3*y
p3 = - 4*k^2 + 2*d1^2*d2^2	    % x^2*y^2
p4 = 2*d1^2*k^2			    % x^2
p5 = 0                              % x*y^3
p6 = 0				    % x*y
p7 = -d2^4			    % y^4
p8 = 2*d2^2*k^2			    % y^2
p9 = -k^4			    % free

eq1 = A1*A2 == p1		    % x^4
eq2 = A1*C2 + A2*C1 == p2	    % x^3*y
eq3 = A1*B2 + A2*B1 + C1*C2 == p3   % x^2*y^2
eq4 = A1*D2 + A2*D1 == p4	    % x^2
eq5 = B1*C2 + B2*C1 == p5           % x*y^3
eq6 = C1*D2 + C2*D1 == p6           % x*y
eq7 = B1*B2 == p7                   % y^4
eq8 = B1*D2 + B2*D1 == p8           % y^2
eq9 = D1*D2 == p9                   % free

% If the term that multiplies d1^4 is negative, negate all.
% This guarantees some consistency.

term = children(eq1)
[left right] = term{:}
negate = false
[m n]=size(eq1)
for i=1:m
    if has(right(i), d1^4)
        if isAlways(right(i) <= 0)
            negate = true
        end
        break
    end
end

if negate
    eq1 = A1*A2 == -p1		    	% x^4
    eq2 = A1*C2 + A2*C1 == -p2	    	% x^3*y
    eq3 = A1*B2 + A2*B1 + C1*C2 == -p3  % x^2*y^2
    eq4 = A1*D2 + A2*D1 == -p4	    	% x^2
    eq5 = B1*C2 + B2*C1 == -p5          % x*y^3
    eq6 = C1*D2 + C2*D1 == -p6          % x*y
    eq7 = B1*B2 == -p7                  % y^4
    eq8 = B1*D2 + B2*D1 == -p8          % y^2
    eq9 = D1*D2 == -p9                  % free
end

fid_c_oa = fopen('sol_c_oa.txt', 'wt')

eq10 = A1 == d1^2
eq13 = D1 == -k^2
eq14 = A2 == d2^2
eq17 = D2 == -k^2

sol_c = solve([eq1, eq2, eq3, eq4, eq5, eq6, eq7, eq8, eq9, eq13], [A1 B1 C1 D1 A2 B2 C2 D2], 'Real', true, 'ReturnConditions', true, 'IgnoreAnalyticConstraints', true, 'MaxDegree', 2)
A1 = reduceRedundancies([sol_c.A1], [d1, d2, k, m2])
fprintf(fid_c_oa, 'A1 = %s\n', char(A1))
B1 = reduceRedundancies([sol_c.B1], [d1, d2, k, m2])
fprintf(fid_c_oa, 'B1 = %s\n', char(B1))
C1 = reduceRedundancies([sol_c.C1], [d1, d2, k, m2])
fprintf(fid_c_oa, 'C1 = %s\n', char(C1))
D1 = reduceRedundancies([sol_c.D1], [d1, d2, k, m2])
fprintf(fid_c_oa, 'D1 = %s\n', char(D1))

fprintf(fid_c_oa, '\n')
A2 = reduceRedundancies([sol_c.A2], [d1, d2, k, m2])
fprintf(fid_c_oa, 'A2 = %s\n', char(A2))
B2 = reduceRedundancies([sol_c.B2], [d1, d2, k, m2])
fprintf(fid_c_oa, 'B2 = %s\n', char(B2))
C2 = reduceRedundancies([sol_c.C2], [d1, d2, k, m2])
fprintf(fid_c_oa, 'C2 = %s\n', char(C2))
D2 = reduceRedundancies([sol_c.D2], [d1, d2, k, m2])
fprintf(fid_c_oa, 'D2 = %s\n', char(D2))

fclose(fid_c_oa)
