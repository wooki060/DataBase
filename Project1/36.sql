select distinct T.name
from Trainer as T, CatchedPokemon as C
where C.pid in (
  select after_id
  from Evolution
  where after_id not in (
    select before_id
    from Evolution
    )
  )
and T.id = C.owner_id
order by T.name
