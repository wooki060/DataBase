select T.name, count(C.owner_id)
from Trainer as T, CatchedPokemon as  C
where T.hometown = 'Sangnok City'
and T.id = C.owner_id
group by C.owner_id
order by count(C.owner_id)
