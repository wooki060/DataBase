select name
from Pokemon
where id in (
  select pid
  from CatchedPokemon
  where owner_id in (
    select id
    from Trainer
    where hometown = 'Sangnok City'
  )
)
and id in(
  select pid
  from CatchedPokemon
  where owner_id in (
    select id
    from Trainer
    where hometown = 'Brown City'
  )
)
order by name
  
  

  
  
