select name, avg(level)
from Trainer as T, CatchedPokemon as C
where T.id = C.owner_id
group by name
order by name
