select P.name
from CatchedPokemon as C, Pokemon as P
where C.nickname like '% %'
and C.pid = P.id
order by P.name desc
